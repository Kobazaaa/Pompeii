// -- Standard Library --
#include <array>
#include <iostream>

// -- Pompeii Includes --
#include "ForwardPass.h"
#include "ShadowPass.h"
#include "Debugger.h"
#include "Shader.h"
#include "DescriptorPool.h"
#include "Context.h"
#include "Camera.h"
#include "ConsoleTextSettings.h"
#include "Scene.h"

void pom::ForwardPass::Initialize(const Context& context, const ForwardPassCreateInfo& createInfo)
{
	// -- MSAA Image --
	{
		CreateMSAAImage(context, createInfo.extent, createInfo.format);
		m_DeletionQueue.Push([&] { m_MSAAImage.Destroy(context); });
	}

	// -- Descriptor Set Layout --
	{
		DescriptorSetLayoutBuilder builder{};

		// -- Uniform Buffer Descriptor --
		builder
			.SetDebugName("Uniform Buffer DS Layout")
			.NewLayoutBinding()
			.SetType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			.SetShaderStages(VK_SHADER_STAGE_VERTEX_BIT)
			.Build(context, m_UniformDSL);
		m_DeletionQueue.Push([&] { m_UniformDSL.Destroy(context); });

		// -- Texture Array Descriptor --
		builder
			.SetDebugName("Texture Array DS Layout")
			.NewLayoutBinding()
			.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.SetCount(static_cast<uint32_t>(createInfo.pScene->model.images.size()))
			.Build(context, m_TextureDSL);
		m_DeletionQueue.Push([&] { m_TextureDSL.Destroy(context); });

		// -- Shadow Map --
		builder
			.SetDebugName("LightMap")
			.NewLayoutBinding()
			.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(context, m_ShadowMapDSL);
		m_DeletionQueue.Push([&] { m_ShadowMapDSL.Destroy(context); });

		// -- Light Data --
		builder
			.SetDebugName("Light Data")
			.NewLayoutBinding()
			.SetType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(context, m_LightDSL);
		m_DeletionQueue.Push([&] { m_LightDSL.Destroy(context); });
	}

	// -- Pipeline Layout --
	{
		GraphicsPipelineLayoutBuilder builder{};

		builder
			.NewPushConstantRange()
				.SetPCOffset(0)
				.SetPCSize(sizeof(PCModelDataVS))
				.SetPCStageFlags(VK_SHADER_STAGE_VERTEX_BIT)
			.NewPushConstantRange()
				.SetPCOffset(sizeof(PCModelDataVS))
				.SetPCSize(sizeof(PCMaterialDataFS))
				.SetPCStageFlags(VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddLayout(m_UniformDSL)
			.AddLayout(m_TextureDSL)
			.AddLayout(m_ShadowMapDSL)
			.AddLayout(m_LightDSL)
			.Build(context, m_DefaultPipelineLayout);
		m_DeletionQueue.Push([&] {m_DefaultPipelineLayout.Destroy(context); });
	}

	// -- Pipelines --
	{
		// Load in shaders
		ShaderLoader shaderLoader{};
		ShaderModule vertShader;
		ShaderModule fragShader;
		shaderLoader.Load(context, "shaders/shader.vert.spv", vertShader);
		shaderLoader.Load(context, "shaders/shader.frag.spv", fragShader);
		uint32_t arraySize = static_cast<uint32_t>(createInfo.pScene->model.images.size());

		// Setup dynamic rendering info
		VkPipelineRenderingCreateInfo renderingCreateInfo{};
		VkFormat format = createInfo.format;
		renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingCreateInfo.colorAttachmentCount = 1;
		renderingCreateInfo.pColorAttachmentFormats = &format;
		renderingCreateInfo.depthAttachmentFormat = createInfo.depthFormat;

		// Create pipeline
		GraphicsPipelineBuilder builder{};
		builder
			.SetDebugName("Graphics Pipeline (Default)")
			.SetPipelineLayout(m_DefaultPipelineLayout)
			.SetupDynamicRendering(renderingCreateInfo)
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.AddShader(vertShader, VK_SHADER_STAGE_VERTEX_BIT)
			.AddShader(fragShader, VK_SHADER_STAGE_FRAGMENT_BIT)
				.SetShaderSpecialization(0, 0, sizeof(uint32_t), &arraySize)
			.EnableSampleShading(0.2f)
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetCullMode(VK_CULL_MODE_BACK_BIT)
			.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.SetPolygonMode(VK_POLYGON_MODE_FILL)
			.SetSampleCount(context.physicalDevice.GetMaxSampleCount())
			.SetDepthTest(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL)
			.SetVertexBindingDesc(Vertex::GetBindingDescription())
			.SetVertexAttributeDesc(Vertex::GetAttributeDescriptions())
			.Build(context, m_OpaquePipeline);
		m_DeletionQueue.Push([&] { m_OpaquePipeline.Destroy(context); });

		builder = {};

		builder
			.SetDebugName("Graphics Pipeline (Transparent)")
			.SetPipelineLayout(m_DefaultPipelineLayout)
			.SetupDynamicRendering(renderingCreateInfo)
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.AddShader(vertShader, VK_SHADER_STAGE_VERTEX_BIT)
			.AddShader(fragShader, VK_SHADER_STAGE_FRAGMENT_BIT)
				.SetShaderSpecialization(0, 0, sizeof(uint32_t), &arraySize)
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetCullMode(VK_CULL_MODE_NONE)
			.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.SetPolygonMode(VK_POLYGON_MODE_FILL)
			.SetSampleCount(context.physicalDevice.GetMaxSampleCount())
			.SetDepthTest(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL)
			.SetVertexBindingDesc(Vertex::GetBindingDescription())
			.SetVertexAttributeDesc(Vertex::GetAttributeDescriptions())
			.EnableBlend(0)
				.SetColorBlend(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD)
				.SetAlphaBlend(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD)
			.Build(context, m_TransparentPipeline);
		m_DeletionQueue.Push([&] { m_TransparentPipeline.Destroy(context); });

		fragShader.Destroy(context);
		vertShader.Destroy(context);
	}

	// -- Sampler --
	{
		SamplerBuilder builder{};
		builder
			.SetFilters(VK_FILTER_LINEAR, VK_FILTER_LINEAR)
			.SetAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
			.EnableAnisotropy(context.physicalDevice.GetProperties().limits.maxSamplerAnisotropy)
			.SetMipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
			.SetMipLevels(0.f, 0.f, VK_LOD_CLAMP_NONE)
			.SetBorderColor(VK_BORDER_COLOR_INT_OPAQUE_BLACK)
			.Build(context, m_Sampler);
		m_DeletionQueue.Push([&] { m_Sampler.Destroy(context); });
	}

	// -- UBO --
	{
		m_vUniformBuffers.resize(createInfo.maxFramesInFlight);
		m_vLightBuffers.resize(createInfo.maxFramesInFlight);
		for (size_t i{}; i < createInfo.maxFramesInFlight; ++i)
		{
			BufferAllocator bufferAlloc{};
			bufferAlloc
				.SetDebugName("Uniform Buffer (Matrices)")
				.SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
				.SetSize(sizeof(UniformBufferVS))
				.HostAccess(true)
				.Allocate(context, m_vUniformBuffers[i]);
			bufferAlloc = {};
			bufferAlloc
				.SetDebugName("Uniform Buffer (Light)")
				.SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
				.SetSize(sizeof(UniformBufferFS))
				.HostAccess(true)
				.Allocate(context, m_vLightBuffers[i]);
		}
		m_DeletionQueue.Push([&] { for (auto& ubo : m_vUniformBuffers) ubo.Destroy(context); });
		m_DeletionQueue.Push([&] { for (auto& ubo : m_vLightBuffers) ubo.Destroy(context); });
	}

	// -- Buffers --
	{
		m_vUniformDS = createInfo.pDescriptorPool->AllocateSets(context, m_UniformDSL, createInfo.maxFramesInFlight, "Uniform Buffer DS");
		m_TextureDS = createInfo.pDescriptorPool->AllocateSets(context, m_TextureDSL, 1, "Texture Array DS").front();
		m_ShadowMapDS = createInfo.pDescriptorPool->AllocateSets(context, m_ShadowMapDSL, createInfo.maxFramesInFlight, "Shadow Map DS");
		m_vLightDS = createInfo.pDescriptorPool->AllocateSets(context, m_LightDSL, createInfo.maxFramesInFlight, "Light DS");

		// -- Write UBO --
		DescriptorSetWriter writer{};
		for (size_t i{}; i < createInfo.maxFramesInFlight; ++i)
		{
			writer
				.AddBufferInfo(m_vUniformBuffers[i], 0, sizeof(UniformBufferVS))
				.WriteBuffers(m_vUniformDS[i], 0)
				.Execute(context);

			writer
				.AddImageInfo(createInfo.pShadowPass->GetMap(static_cast<uint32_t>(i)),
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, createInfo.pShadowPass->GetSampler())
				.WriteImages(m_ShadowMapDS[i], 0)
				.Execute(context);

			writer
				.AddBufferInfo(m_vLightBuffers[i], 0, sizeof(UniformBufferFS))
				.WriteBuffers(m_vLightDS[i], 0)
				.Execute(context);
		}

		// -- Write Textures --
		for (Image& image : createInfo.pScene->model.images)
		{
			writer.AddImageInfo(image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_Sampler);
		}
		writer.WriteImages(m_TextureDS, 0).Execute(context);
	}
}

void pom::ForwardPass::Destroy()
{
	m_DeletionQueue.Flush();
}

void pom::ForwardPass::Resize(const Context& context, VkExtent2D extent, VkFormat format)
{
	m_MSAAImage.Destroy(context);
	CreateMSAAImage(context, extent, format);
}
void pom::ForwardPass::Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, Image& recordImage, Image& depthImage, Scene* pScene, Camera* pCamera)
{
	// Do sizes match?!
	VkExtent3D extentImage = recordImage.GetExtent3D();
	VkExtent3D extentMSAA = m_MSAAImage.GetExtent3D();
	if (extentImage.width  != extentMSAA.width  ||
		extentImage.height != extentMSAA.height || 
		extentImage.depth  != extentMSAA.depth)
	{
		std::cout << WARNING_TXT << "RecordImage and MSAA Image have different sizes and thus are incompatible! Did you forget to resize?\n" << RESET_TXT;
		return;
	}

	// Update VS UBO
	UniformBufferVS ubo;
	ubo.view = pCamera->GetViewMatrix();
	ubo.proj = pCamera->GetProjectionMatrix();
	ubo.lightSpace = pScene->directionalLight.GetLightSpaceMatrix();
	vmaCopyMemoryToAllocation(context.allocator, &ubo, m_vUniformBuffers[imageIndex].GetMemoryHandle(), 0, sizeof(ubo));

	// Update FS UBO
	UniformBufferFS ubofs;
	ubofs.intensity = pScene->directionalLight.GetIntensity();
	ubofs.color = pScene->directionalLight.GetColor();
	ubofs.dir = pScene->directionalLight.GetDirection();
	vmaCopyMemoryToAllocation(context.allocator, &ubofs, m_vLightBuffers[imageIndex].GetMemoryHandle(), 0, sizeof(ubofs));

	// Transition Depth Image
	//depthImage.TransitionLayout(commandBuffer,
	//	VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
	//	VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
	//	VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
	//	0, 1, 0, 1);

	// Transition MSAA Image
	m_MSAAImage.TransitionLayout(commandBuffer,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
		VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		0, 1, 0, 1);

	// Transition Swapchain Image
	recordImage.TransitionLayout(commandBuffer,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
		VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		0, 1, 0, 1);

	// Setup attachments
	VkRenderingAttachmentInfo colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView = m_MSAAImage.GetViewHandle();
	colorAttachment.imageLayout = m_MSAAImage.GetCurrentLayout();
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.clearValue.color = { {0.53f, 0.81f, 0.92f, 1.0f} };
	colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
	colorAttachment.resolveImageLayout = recordImage.GetCurrentLayout();
	colorAttachment.resolveImageView = recordImage.GetViewHandle();


	VkRenderingAttachmentInfo depthAttachment{};
	depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depthAttachment.imageView = depthImage.GetViewHandle();
	depthAttachment.imageLayout = depthImage.GetCurrentLayout();
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// Render Info
	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea = VkRect2D{ VkOffset2D{0, 0}, recordImage.GetExtent2D() };
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;
	renderingInfo.pDepthAttachment = &depthAttachment;
	renderingInfo.pStencilAttachment = nullptr;

	// Render
	const VkCommandBuffer& vCmdBuffer = commandBuffer.GetHandle();
	Debugger::BeginDebugLabel(commandBuffer, "Render Pass", glm::vec4(0.6f, 0.2f, 0.8f, 1));
	vkCmdBeginRendering(vCmdBuffer, &renderingInfo);
	{
		// -- Set Dynamic Viewport --
		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(recordImage.GetExtent2D().width);
		viewport.height = static_cast<float>(recordImage.GetExtent2D().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		Debugger::InsertDebugLabel(commandBuffer, "Bind Viewport", glm::vec4(0.2f, 1.f, 0.2f, 1.f));
		vkCmdSetViewport(vCmdBuffer, 0, 1, &viewport);

		// -- Set Dynamic Scissors --
		VkRect2D scissor;
		scissor.offset = { .x = 0, .y = 0 };
		scissor.extent = recordImage.GetExtent2D();
		Debugger::InsertDebugLabel(commandBuffer, "Bind Scissor", glm::vec4(1.f, 1.f, 0.2f, 1.f));
		vkCmdSetScissor(vCmdBuffer, 0, 1, &scissor);

		// -- Bind Descriptor Sets --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Uniform Buffer", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DefaultPipelineLayout.GetHandle(), 0, 1, &m_vUniformDS[imageIndex].GetHandle(), 0, nullptr);

		Debugger::InsertDebugLabel(commandBuffer, "Bind Textures", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DefaultPipelineLayout.GetHandle(), 1, 1, &m_TextureDS.GetHandle(), 0, nullptr);

		Debugger::InsertDebugLabel(commandBuffer, "Bind Shadow Map", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DefaultPipelineLayout.GetHandle(), 2, 1, &m_ShadowMapDS[imageIndex].GetHandle(), 0, nullptr);

		Debugger::InsertDebugLabel(commandBuffer, "Bind Light Ubo", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DefaultPipelineLayout.GetHandle(), 3, 1, &m_vLightDS[imageIndex].GetHandle(), 0, nullptr);

		// -- Bind Model Data --
		pScene->model.Bind(commandBuffer);

		// -- Draw Opaque --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Pipeline (Default)", glm::vec4(0.2f, 0.4f, 1.f, 1.f));
		vkCmdBindPipeline(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_OpaquePipeline.GetHandle());
		for (const Mesh& mesh : pScene->model.opaqueMeshes)
		{
			// -- Bind Push Constants --
			Debugger::InsertDebugLabel(commandBuffer, "Push Constants", glm::vec4(1.f, 0.6f, 0.f, 1.f));
			PCModelDataVS pcvs
			{
				.model = mesh.matrix
			};
			vkCmdPushConstants(vCmdBuffer, m_DefaultPipelineLayout.GetHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
			                   sizeof(PCModelDataVS), &pcvs);

			PCMaterialDataFS pcfs
			{
				.diffuseIdx = mesh.material.diffuseIdx,
				.opacityIdx = mesh.material.opacityIdx,
				.specularIdx = mesh.material.specularIdx,
				.shininessIdx = mesh.material.shininessIdx,
				.heightIdx = mesh.material.heightIdx,
				.expo = mesh.material.exp
			};
			vkCmdPushConstants(vCmdBuffer, m_DefaultPipelineLayout.GetHandle(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PCModelDataVS),
			                   sizeof(PCMaterialDataFS), &pcfs);

			// -- Drawing Time! --
			vkCmdDrawIndexed(vCmdBuffer, mesh.indexCount, 1, mesh.indexOffset, mesh.vertexOffset, 0);
			Debugger::InsertDebugLabel(commandBuffer, "Draw Opaque Mesh - " + mesh.name, glm::vec4(0.4f, 0.8f, 1.f, 1.f));
		}

		// -- Draw Transparent --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Pipeline (Transparent)", glm::vec4(0.2f, 0.4f, 1.f, 1.f));
		vkCmdBindPipeline(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_TransparentPipeline.GetHandle());
		for (const Mesh& mesh : pScene->model.transparentMeshes)
		{
			// -- Bind Push Constants --
			Debugger::InsertDebugLabel(commandBuffer, "Push Constants", glm::vec4(1.f, 0.6f, 0.f, 1.f));
			PCModelDataVS pcvs
			{
				.model = mesh.matrix
			};
			vkCmdPushConstants(vCmdBuffer, m_DefaultPipelineLayout.GetHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
				sizeof(PCModelDataVS), &pcvs);

			PCMaterialDataFS pcfs
			{
				.diffuseIdx = mesh.material.diffuseIdx,
				.opacityIdx = mesh.material.opacityIdx,
				.specularIdx = mesh.material.specularIdx,
				.shininessIdx = mesh.material.shininessIdx,
				.heightIdx = mesh.material.heightIdx,
				.expo = mesh.material.exp
			};
			vkCmdPushConstants(vCmdBuffer, m_DefaultPipelineLayout.GetHandle(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PCModelDataVS),
				sizeof(PCMaterialDataFS), &pcfs);

			// -- Drawing Time! --
			vkCmdDrawIndexed(vCmdBuffer, mesh.indexCount, 1, mesh.indexOffset, mesh.vertexOffset, 0);
			Debugger::InsertDebugLabel(commandBuffer, "Draw Transparent Mesh - " + mesh.name, glm::vec4(0.4f, 0.8f, 1.f, 1.f));
		}
	}
	vkCmdEndRendering(vCmdBuffer);
	Debugger::EndDebugLabel(commandBuffer);
}

void pom::ForwardPass::CreateMSAAImage(const Context& context, VkExtent2D extent, VkFormat format)
{
	ImageBuilder iBuilder{};
	iBuilder
		.SetDebugName("MSAA Buffer")
		.SetWidth(extent.width)
		.SetHeight(extent.height)
		.SetFormat(format)
		.SetMipLevels(1)
		.SetSampleCount(context.physicalDevice.GetMaxSampleCount())
		.SetTiling(VK_IMAGE_TILING_OPTIMAL)
		.SetUsageFlags(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		.Build(context, m_MSAAImage);
	m_MSAAImage.CreateView(context, format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);
}
