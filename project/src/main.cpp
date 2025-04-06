#include <vulkan/vulkan.h>
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <vector>
#include <array>
#include <map>
#include <optional>
#include <set>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <unordered_map>

#include "glm/gtc/quaternion.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "CommandPool.h"
#include "DeletionQueue.h"
#include "Device.h"
#include "Image.h"
#include "Window.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "RenderPass.h"
#include "Shader.h"
#include "SwapChain.h"

const std::string g_MODEL_PATH = "models/viking_room.obj";
const std::string g_TEXTURE_PATH = "textures/viking_room.png";

constexpr int g_MAX_FRAMES_IN_FLIGHT = 2;


struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}
	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const
	{
		return position == other.position &&
			   color == other.color &&
			   texCoord == other.texCoord;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position) ^
					(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
					(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class HelloTriangleApplication
{
public:
	void Run()
	{
		m_pWindow.Initialize(800, 600, "Vulkan Refactored");
		m_DeletionQueue.Push([&] { m_pWindow.Destroy(); });

		InitVulkan();
		MainLoop();
		Cleanup();
	}

private:
	void InitVulkan()
	{
		// --	Enable Debugger	--//
		{
			#ifdef NDEBUG
				pom::Debugger::SetEnabled(false);
			#else
				pom::Debugger::SetEnabled(true);
				pom::Debugger::AddValidationLayer("VK_LAYER_KHRONOS_validation");
			#endif
		}

		// --	Create Instance	--//
		{
			pom::InstanceBuilder builder;

			builder.SetApplicationName("Vulkan Refactored")
				   .SetEngineName("No Engine")
				   .Build(m_Instance);

			m_DeletionQueue.Push([&] { m_Instance.Destroy(); });
		}

		// --	Setup Debugger	--//
		{
			pom::Debugger::SetupMessenger(m_Instance);
			m_DeletionQueue.Push([&] { if (pom::Debugger::IsEnabled()) pom::Debugger::DestroyMessenger(m_Instance); });
		}

		// --	Create Surface	--//
		{
			m_pWindow.CreateVulkanSurface(m_Instance);
			m_DeletionQueue.Push([&] { vkDestroySurfaceKHR(m_Instance.GetInstance(), m_pWindow.GetVulkanSurface(), nullptr); });
		}

		// --	Select GPU	--//
		{
			pom::PhysicalDeviceSelector selector;
			selector.AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
					.PickPhysicalDevice(m_Instance, m_PhysicalDevice, m_pWindow.GetVulkanSurface());
		}

		// --	Create Device		--//
		{
			VkPhysicalDeviceFeatures desiredFeatures{};
			desiredFeatures.samplerAnisotropy = VK_TRUE;

			pom::DeviceBuilder deviceBuilder{};

			deviceBuilder.SetFeatures(desiredFeatures)
						 .Build(m_PhysicalDevice, m_Device);

			m_DeletionQueue.Push([&] { m_Device.Destroy(); });
		}

		// --	Create Allocator 	  --//
		{
			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.physicalDevice = m_PhysicalDevice.GetPhysicalDevice();
			allocatorInfo.device = m_Device.GetDevice();
			allocatorInfo.instance = m_Instance.GetInstance();

			vmaCreateAllocator(&allocatorInfo, &m_Allocator);
			m_DeletionQueue.Push([&] { vmaDestroyAllocator(m_Allocator); });
		}

		// --	Create Command Pool 	  --//
		{
			m_CommandPool.Create(m_Device, m_PhysicalDevice)
						 .AllocateCmdBuffers(g_MAX_FRAMES_IN_FLIGHT);

			m_DeletionQueue.Push([&] { m_CommandPool.Destroy(); });
		}

		// --	Create Swap Chain	  --//
		{
			pom::SwapChainBuilder builder;

			builder.SetDesiredImageCount(2)
				   .SetImageArrayLayers(1)
				   .SetImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
				   .Build(m_Device, m_Allocator, m_PhysicalDevice, m_pWindow, m_SwapChain, m_CommandPool);

			m_DeletionQueue.Push([&]
				{
					for (auto& framebuffer : m_vSwapChainFrameBuffers)
						vkDestroyFramebuffer(m_Device.GetDevice(), framebuffer, nullptr);

					m_SwapChain.Destroy(m_Device, m_Allocator);
				});
		}

		// --	Create Render Pass   --//
		{
			pom::RenderPassBuilder builder{};

			builder
				.NewAttachment()
					.SetFormat(m_SwapChain.GetFormat())
					.SetSamples(VK_SAMPLE_COUNT_1_BIT)
					.SetLoadStoreOp(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
					.SetStencilLoadStoreOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE)
					.SetInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
					.SetFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
					.AddColorAttachment(0)
				.NewAttachment()
					.SetFormat(m_SwapChain.GetDepthImage().GetFormat())
					.SetSamples(VK_SAMPLE_COUNT_1_BIT)
					.SetLoadStoreOp(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE)
					.SetStencilLoadStoreOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE)
					.SetInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
					.SetFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
					.AddDepthAttachment(1)
				.NewSubpass()
					.SetBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS)
				.NewDependency()
					.SetSrcSubPass(VK_SUBPASS_EXTERNAL)
					.SetDstSubPass(0)
					.SetSrcMasks(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0)
					.SetDstMasks(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
								 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
				.Build(m_Device, m_RenderPass);
			m_DeletionQueue.Push([&] { m_RenderPass.Destroy(m_Device); });
		}

		CreateDescriptorSetLayout();
		CreateGraphicsPipeline();
		CreateFrameBuffers();
		CreateTextureImage();
		CreateTextureImageSampler();
		LoadModel();
		CreateVertexBuffer();
		CreateIndexBuffer();
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSets();

		// --	Create Sync Objects	  --//
		{
			m_SyncManager.Create(m_Device, g_MAX_FRAMES_IN_FLIGHT);
			m_DeletionQueue.Push([&] {m_SyncManager.Cleanup(); });
		}
	}

	void MainLoop()
	{
		while (!glfwWindowShouldClose(m_pWindow.GetWindow()))
		{
			glfwPollEvents();
			DrawFrame();
		}

		m_Device.WaitIdle();
	}

	void Cleanup()
	{
		vkDestroyPipeline(m_Device.GetDevice(), m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_Device.GetDevice(), m_PipelineLayout, nullptr);

		for (size_t i{}; i < g_MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(m_Device.GetDevice(), m_vUniformBuffers[i], nullptr);
			vmaFreeMemory(m_Allocator, m_vUniformBuffersMemory[i]);
		}

		vkDestroyDescriptorPool(m_Device.GetDevice(), m_DescriptorPool, nullptr);

		vkDestroySampler(m_Device.GetDevice(), m_TextureSampler, nullptr);

		m_TextureImage.Destroy(m_Device, m_Allocator);

		vkDestroyDescriptorSetLayout(m_Device.GetDevice(), m_DescriptorSetLayout, nullptr);

		vkDestroyBuffer(m_Device.GetDevice(), m_IndexBuffer, nullptr);
		vmaFreeMemory(m_Allocator, m_IndexBufferMemory);

		vkDestroyBuffer(m_Device.GetDevice(), m_VertexBuffer, nullptr);
		vmaFreeMemory(m_Allocator, m_VertexBufferMemory);

		m_DeletionQueue.Flush();
	}

	void CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(m_Device.GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Descriptor Set Layout!");
	}

	void CreateGraphicsPipeline()
	{
		pom::ShaderLoader shaderLoader{};
		pom::ShaderModule vertShader = shaderLoader.Load(m_Device, "shaders/shader.vert.spv");
		pom::ShaderModule fragShader = shaderLoader.Load(m_Device, "shaders/shader.frag.spv");

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShader.GetShader();
		vertShaderStageInfo.pName = "main";
		vertShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShader.GetShader();
		fragShaderStageInfo.pName = "main";
		fragShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescriptions = Vertex::GetAttributeDescriptions();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f;
		depthStencil.maxDepthBounds = 1.0f;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout!");

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = m_RenderPass.GetRenderPass();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(m_Device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Graphics Pipeline!");

		vertShader.Destroy(m_Device);
		fragShader.Destroy(m_Device);
	}

	void CreateFrameBuffers()
	{
		m_vSwapChainFrameBuffers.resize(m_SwapChain.GetImageCount());

		for (size_t idx = 0; idx < m_SwapChain.GetImageCount(); ++idx)
		{
			std::array<VkImageView, 2>  attachments = {
				m_SwapChain.GetImages()[idx].GetImageView(),
				m_SwapChain.GetDepthImage().GetImageView()
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass.GetRenderPass();
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_SwapChain.GetExtent().width;
			framebufferInfo.height = m_SwapChain.GetExtent().height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_Device.GetDevice(), &framebufferInfo, nullptr, &m_vSwapChainFrameBuffers[idx]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create Framebuffer!");
		}
	}

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, bool mappable, VmaMemoryUsage memusage, VkBuffer& buffer, VmaAllocation& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		//if (vkCreateBuffer(m_Device.GetDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		//	throw std::runtime_error("Failed to create Buffer!");

		VmaAllocationCreateInfo allocCreateInfo = {};
		if (mappable)
		{
			allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		}
		else
		{
			allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}
		allocCreateInfo.usage = memusage;
		vmaCreateBuffer(m_Allocator, &bufferInfo, &allocCreateInfo, &buffer, &bufferMemory, nullptr);
	}

	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(m_Device.GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Image View!");

		return imageView;
	}

	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice.GetPhysicalDevice(), format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR &&
				(props.linearTilingFeatures & features) == features)
				return format;
			else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
				(props.optimalTilingFeatures & features) == features)
				return format;
		}

		throw std::runtime_error("Failed to find Supported Format!");
	}

	bool HasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
			   format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void CreateTextureImage()
	{
		int texWidth;
		int texHeight;
		int texChannels;
		stbi_uc* pixels = stbi_load(g_TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels)
			throw std::runtime_error("Failed to load Texture Image!");

		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferMemory;
		CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, VMA_MEMORY_USAGE_AUTO,
			stagingBuffer, stagingBufferMemory);

		vmaCopyMemoryToAllocation(m_Allocator, pixels, stagingBufferMemory, 0, imageSize);

		stbi_image_free(pixels);

		pom::ImageBuilder builder{};
		builder.SetWidth(texWidth)
			.SetHeight(texHeight)
			.SetFormat(VK_FORMAT_R8G8B8A8_SRGB)
			.SetTiling(VK_IMAGE_TILING_OPTIMAL)
			.SetUsageFlags(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.Build(m_Allocator, m_TextureImage);
		m_TextureImage.GenerateImageView(m_Device, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);

		m_CommandPool.TransitionImageLayout(m_TextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		m_CommandPool.CopyBufferToImage(stagingBuffer, m_TextureImage.GetImage(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		m_CommandPool.TransitionImageLayout(m_TextureImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(m_Device.GetDevice(), stagingBuffer, nullptr);
		vmaFreeMemory(m_Allocator, stagingBufferMemory);
	}

	void CreateTextureImageSampler()
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(m_PhysicalDevice.GetPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(m_Device.GetDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Texture Sampler!");
	}

	void LoadModel()
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string error;

		// By default, automatically triangulates
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &error, g_MODEL_PATH.c_str()))
			throw std::runtime_error(error);

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes)
		{
			int counterIdx = 0;
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};

				vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 2],
						attrib.vertices[3 * index.vertex_index + 1]
				};
				vertex.texCoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
				vertex.color = { 1.0f, 1.0f, 1.0f };

				if (!uniqueVertices.contains(vertex))
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(m_vVertices.size());
					m_vVertices.push_back(vertex);
				}
				m_vIndices.push_back(uniqueVertices[vertex]);
				// Swap 2 indices to make each triangle LH with clockwise front
				if (counterIdx % 3 == 2)
					std::swap(m_vIndices[counterIdx - 1], m_vIndices[counterIdx]);
				++counterIdx;
			}
		}
	}

	void CreateVertexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(m_vVertices[0]) * m_vVertices.size();

		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			true, VMA_MEMORY_USAGE_AUTO, stagingBuffer, stagingBufferMemory);

		vmaCopyMemoryToAllocation(m_Allocator, m_vVertices.data(), stagingBufferMemory, 0, bufferSize);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			false , VMA_MEMORY_USAGE_AUTO, m_VertexBuffer, m_VertexBufferMemory);
		m_CommandPool.CopyBufferToBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

		vkDestroyBuffer(m_Device.GetDevice(), stagingBuffer, nullptr);
		vmaFreeMemory(m_Allocator, stagingBufferMemory);
	}

	void CreateIndexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(m_vIndices[0]) * m_vIndices.size();

		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, VMA_MEMORY_USAGE_AUTO, stagingBuffer, stagingBufferMemory);

		vmaCopyMemoryToAllocation(m_Allocator, m_vIndices.data(), stagingBufferMemory, 0, bufferSize);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, false, VMA_MEMORY_USAGE_AUTO, m_IndexBuffer, m_IndexBufferMemory);
		m_CommandPool.CopyBufferToBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

		vmaFreeMemory(m_Allocator, stagingBufferMemory);
		vkDestroyBuffer(m_Device.GetDevice(), stagingBuffer, nullptr);
	}

	void CreateUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		m_vUniformBuffers.resize(g_MAX_FRAMES_IN_FLIGHT);
		m_vUniformBuffersMemory.resize(g_MAX_FRAMES_IN_FLIGHT);

		for (size_t i{}; i < g_MAX_FRAMES_IN_FLIGHT; ++i)
		{
			CreateBuffer(bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true,
				VMA_MEMORY_USAGE_AUTO,
				m_vUniformBuffers[i], m_vUniformBuffersMemory[i]);
		}
	}

	void CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(g_MAX_FRAMES_IN_FLIGHT);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(g_MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(g_MAX_FRAMES_IN_FLIGHT);
		poolInfo.flags = 0;

		if (vkCreateDescriptorPool(m_Device.GetDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Descriptor Pool!");
	}

	void CreateDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(g_MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(g_MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		m_vDescriptorSets.resize(g_MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(m_Device.GetDevice(), &allocInfo, m_vDescriptorSets.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate Descriptor Sets!");

		for (size_t i{}; i < g_MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_vUniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = m_TextureImage.GetCurrentLayout();
			imageInfo.imageView = m_TextureImage.GetImageView();
			imageInfo.sampler = m_TextureSampler;

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = m_vDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			descriptorWrites[0].pImageInfo = nullptr;
			descriptorWrites[0].pTexelBufferView = nullptr;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = m_vDescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;
			descriptorWrites[1].pBufferInfo = nullptr;
			descriptorWrites[1].pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(m_Device.GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice.GetPhysicalDevice(), &memProperties);

		for (uint32_t idx{0}; idx < memProperties.memoryTypeCount; ++idx)
			if ((typeFilter & (1 << idx)) && (memProperties.memoryTypes[idx].propertyFlags & properties) == properties)
					return idx;

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void RecordCommandBuffer(pom::CommandBuffer& commandBuffer, uint32_t imageIndex)
	{
		VkCommandBuffer vCmdBuffer = commandBuffer.GetBuffer();
		commandBuffer.Begin(0);
		{
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_RenderPass.GetRenderPass();
			renderPassInfo.framebuffer = m_vSwapChainFrameBuffers[imageIndex];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_SwapChain.GetExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
			clearValues[1].depthStencil = { 1.0f, 0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(vCmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdBindPipeline(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = static_cast<float>(m_SwapChain.GetExtent().width);
				viewport.height = static_cast<float>(m_SwapChain.GetExtent().height);
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(vCmdBuffer, 0, 1, &viewport);

				VkRect2D scissor{};
				scissor.offset = { 0, 0 };
				scissor.extent = m_SwapChain.GetExtent();
				vkCmdSetScissor(vCmdBuffer, 0, 1, &scissor);

				VkBuffer vertexBuffers[] = { m_VertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(vCmdBuffer, 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(vCmdBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

				vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_vDescriptorSets[m_CurrentFrame], 0, nullptr);

				vkCmdDrawIndexed(vCmdBuffer, static_cast<uint32_t>(m_vIndices.size()), 1, 0, 0, 0);
			}
			vkCmdEndRenderPass(vCmdBuffer);
		}
		commandBuffer.End();
	}

	void UpdateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.f), glm::vec3(0.0f, 1.0f, 0.0f));
		ubo.view = glm::lookAtLH(glm::vec3(2.0f, 2.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		const float aspectRatio = static_cast<float>(m_SwapChain.GetExtent().width) / static_cast<float>(m_SwapChain.GetExtent().height);
		ubo.proj = glm::perspectiveLH(glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

		vmaCopyMemoryToAllocation(m_Allocator, &ubo, m_vUniformBuffersMemory[currentImage], 0, sizeof(ubo));
	}

	void DrawFrame()
	{
		const auto& frameSync = m_SyncManager.GetFrameSync(m_CurrentFrame);
		vkWaitForFences(m_Device.GetDevice(), 1, &frameSync.inFlight, VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_Device.GetDevice(), m_SwapChain.GetSwapChain(), UINT64_MAX, frameSync.imageAvailable, VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			m_SwapChain.Recreate(m_Device, m_Allocator, m_PhysicalDevice, m_pWindow, m_CommandPool);
			return;
		}
		if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to acquire Swap Chain Image");

		UpdateUniformBuffer(m_CurrentFrame);

		vkResetFences(m_Device.GetDevice(), 1, &frameSync.inFlight);

		pom::CommandBuffer& cmdBuffer = m_CommandPool.GetBuffer(m_CurrentFrame);
		cmdBuffer.Reset();
		RecordCommandBuffer(cmdBuffer, imageIndex);

		const pom::SemaphoreInfo semaphoreInfo
		{
			.vWaitSemaphores	= { frameSync.imageAvailable },
			.vWaitStages		= { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },
			.vSignalSemaphores	= { frameSync.renderFinished }
		};
		cmdBuffer.Submit(m_Device.GetGraphicQueue(), false, semaphoreInfo, frameSync.inFlight);

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = static_cast<uint32_t>(semaphoreInfo.vSignalSemaphores.size());
		presentInfo.pWaitSemaphores = semaphoreInfo.vSignalSemaphores.data();

		VkSwapchainKHR swapChains[] = { m_SwapChain.GetSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		result = vkQueuePresentKHR(m_Device.GetPresentQueue(), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_pWindow.IsOutdated())
		{
			m_pWindow.ResetOutdated();
			m_SwapChain.Recreate(m_Device, m_Allocator, m_PhysicalDevice, m_pWindow, m_CommandPool);
		}
		else if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to present Swap Chain Image!");

		m_CurrentFrame = (m_CurrentFrame + 1) % g_MAX_FRAMES_IN_FLIGHT;
	}


	pom::Window						m_pWindow					{ };

	pom::Instance					m_Instance					{ };
	pom::PhysicalDevice				m_PhysicalDevice			{ };
	pom::Device						m_Device					{ };

	pom::SwapChain					m_SwapChain					{ };

	std::vector<VkFramebuffer>		m_vSwapChainFrameBuffers	{ };

	pom::Image						m_TextureImage				{ };
	VkSampler						m_TextureSampler			{ VK_NULL_HANDLE };

	pom::RenderPass					m_RenderPass				{ };
	VkDescriptorSetLayout			m_DescriptorSetLayout		{ VK_NULL_HANDLE };
	VkPipelineLayout				m_PipelineLayout			{ VK_NULL_HANDLE };
	VkPipeline						m_GraphicsPipeline			{ VK_NULL_HANDLE };

	VkDescriptorPool				m_DescriptorPool			{ VK_NULL_HANDLE };
	std::vector<VkDescriptorSet>	m_vDescriptorSets			{ VK_NULL_HANDLE };

	pom::CommandPool				m_CommandPool				{ };
	pom::SyncManager				m_SyncManager				{ };


	std::vector<Vertex>				m_vVertices;
	std::vector<uint32_t>			m_vIndices;
	VkBuffer						m_VertexBuffer				{ VK_NULL_HANDLE };
	VmaAllocation					m_VertexBufferMemory		{ VK_NULL_HANDLE };
	VkBuffer						m_IndexBuffer				{ VK_NULL_HANDLE };
	VmaAllocation					m_IndexBufferMemory			{ VK_NULL_HANDLE };

	std::vector<VkBuffer>			m_vUniformBuffers			{ VK_NULL_HANDLE };
	std::vector<VmaAllocation>		m_vUniformBuffersMemory		{ VK_NULL_HANDLE };

	uint32_t						m_CurrentFrame				{ 0 };

	VmaAllocator m_Allocator									{ VK_NULL_HANDLE };
	pom::DeletionQueue				m_DeletionQueue				{ };
};

int main()
{
	HelloTriangleApplication app{};

	try
	{
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
