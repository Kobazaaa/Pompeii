// -- Model Includes --
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// -- Container Includes --
#include <array>

// -- Custom Includes --
#include "Renderer.h"
#include "Window.h"
#include "Shader.h"

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::Renderer::Initialize(Window* pWindow)
{
	m_pWindow = pWindow;
	InitializeVulkan();
	LoadModels();
}

void pom::Renderer::Destroy()
{
	m_Device.WaitIdle();
	m_DeletionQueueSC.Flush();
	m_DeletionQueue.Flush();
}


//--------------------------------------------------
//    Loop
//--------------------------------------------------
void pom::Renderer::Update()
{
	// -- Update UBO --
	UpdateUniformBuffer(m_CurrentFrame);
}
void pom::Renderer::Render()
{
	// -- Wait for the current frame to be done --
	const auto& frameSync = m_SyncManager.GetFrameSync(m_CurrentFrame);
	vkWaitForFences(m_Device.GetDevice(), 1, &frameSync.inFlight, VK_TRUE, UINT64_MAX);

	// -- Acquire new Image from SwapChain --
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_Device.GetDevice(), m_SwapChain.GetSwapChain(), UINT64_MAX, frameSync.imageAvailable, VK_NULL_HANDLE, &imageIndex);

	// -- If SwapChain Image not good, recreate Swap Chain --
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		RecreateSwapChain();
		return;
	}
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to acquire Swap Chain Image");

	// -- Reset Fence to be un-signaled (not done) --
	vkResetFences(m_Device.GetDevice(), 1, &frameSync.inFlight);

	// -- Record Command Buffer --
	pom::CommandBuffer& cmdBuffer = m_CommandPool.GetBuffer(m_CurrentFrame);
	cmdBuffer.Reset();
	RecordCommandBuffer(cmdBuffer, imageIndex);

	// -- Submit Commands with Semaphores --
	const pom::SemaphoreInfo semaphoreInfo
	{
		.vWaitSemaphores = { frameSync.imageAvailable },
		.vWaitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },
		.vSignalSemaphores = { frameSync.renderFinished }
	};
	cmdBuffer.Submit(m_Device.GetGraphicQueue(), false, semaphoreInfo, frameSync.inFlight);

	// -- Create Present Info --
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = static_cast<uint32_t>(semaphoreInfo.vSignalSemaphores.size());
	presentInfo.pWaitSemaphores = semaphoreInfo.vSignalSemaphores.data();

	VkSwapchainKHR swapChains[] = { m_SwapChain.GetSwapChain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	// -- Present --
	result = vkQueuePresentKHR(m_Device.GetPresentQueue(), &presentInfo);

	// -- If Present failed or out of date, recreate SwapChain --
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_pWindow->IsOutdated())
	{
		m_pWindow->ResetOutdated();
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to present Swap Chain Image!");

	// -- Go to next frame --
	m_CurrentFrame = (m_CurrentFrame + 1) % g_MAX_FRAMES_IN_FLIGHT;
}


//--------------------------------------------------
//    Vulkan Specific
//--------------------------------------------------
void pom::Renderer::InitializeVulkan()
{
	// -- Enable Debugger - Requirements - [Debug Mode]
	{
#ifdef NDEBUG
		pom::Debugger::SetEnabled(false);
#else
		pom::Debugger::SetEnabled(true);
		pom::Debugger::AddValidationLayer("VK_LAYER_KHRONOS_validation");
#endif
	}

	// -- Create Instance - Requirements - []
	{
		pom::InstanceBuilder builder;

		builder.SetApplicationName("Vulkan Refactored")
			.SetEngineName("No Engine")
			.Build(m_Instance);

		m_DeletionQueue.Push([&] { m_Instance.Destroy(); });
	}

	// -- Setup Debugger - Requirements - [Instance]
	{
		pom::Debugger::SetupMessenger(m_Instance);
		m_DeletionQueue.Push([&] { pom::Debugger::DestroyMessenger(m_Instance); });
	}

	// -- Create Surface - Requirements - [Window - Instance]
	{
		m_pWindow->CreateVulkanSurface(m_Instance);
		m_DeletionQueue.Push([&] { vkDestroySurfaceKHR(m_Instance.GetInstance(), m_pWindow->GetVulkanSurface(), nullptr); });
	}

	// -- Select GPU - Requirements - [Window - Instance]
	{
		pom::PhysicalDeviceSelector selector;
		selector.AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
			.PickPhysicalDevice(m_Instance, m_PhysicalDevice, m_pWindow->GetVulkanSurface());
	}

	// -- Create Device - Requirements - [Physical Device - Instance]
	{
		VkPhysicalDeviceFeatures desiredFeatures{};
		desiredFeatures.samplerAnisotropy = VK_TRUE;

		pom::DeviceBuilder deviceBuilder{};

		deviceBuilder.SetFeatures(desiredFeatures)
			.Build(m_PhysicalDevice, m_Device);

		m_DeletionQueue.Push([&] { m_Device.Destroy(); });
	}

	// -- Create Allocator - Requirements - [Device - Physical Device - Instance]
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = m_PhysicalDevice.GetPhysicalDevice();
		allocatorInfo.device = m_Device.GetDevice();
		allocatorInfo.instance = m_Instance.GetInstance();

		vmaCreateAllocator(&allocatorInfo, &m_Allocator);
		m_DeletionQueue.Push([&] { vmaDestroyAllocator(m_Allocator); });
	}

	// -- Create Command Pool - Requirements - [Device - Physical Device]
	{
		m_CommandPool.Create(m_Device, m_PhysicalDevice)
			.AllocateCmdBuffers(g_MAX_FRAMES_IN_FLIGHT);

		m_DeletionQueue.Push([&] { m_CommandPool.Destroy(); });
	}

	// -- Create SwapChain - Requirements - [Device - Allocator - Physical Device, Window, Command Pool]
	{
		pom::SwapChainBuilder builder;

		builder.SetDesiredImageCount(2)
			.SetImageArrayLayers(1)
			.SetImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			.Build(m_Device, m_Allocator, m_PhysicalDevice, *m_pWindow, m_SwapChain, m_CommandPool);

		m_DeletionQueueSC.Push([&] { m_SwapChain.Destroy(m_Device, m_Allocator); });
	}

	// -- Create Render Pass - Requirements - [Device - SwapChain - Depth Buffer]
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

	// -- Create Descriptor Set Layout - Requirements - [Device]
	{
		pom::DescriptorSetLayoutBuilder builder{};

		builder
			.NewLayoutBinding()
				.SetType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				.SetShaderStages(VK_SHADER_STAGE_VERTEX_BIT)
			.NewLayoutBinding()
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(m_Device, m_DescriptorSetLayout);

		m_DeletionQueue.Push([&] { m_DescriptorSetLayout.Destroy(m_Device); });
	}

	// -- Create Graphics Pipeline Layout - Requirements - [Device - Descriptor Set Layout]
	{
		pom::GraphicsPipelineLayoutBuilder builder{};

		builder
			.AddLayout(m_DescriptorSetLayout)
			.Build(m_Device, m_PipelineLayout);

		m_DeletionQueue.Push([&] {m_PipelineLayout.Destroy(m_Device); });
	}

	// -- Create Graphics Pipeline - Requirements - [Pipeline Layout - Shaders - RenderPass]
	{
		pom::ShaderLoader shaderLoader{};
		auto shaders = shaderLoader.LoadMultiple(m_Device,
			{
				"shader.vert.spv",
				"shader.frag.spv"
			},
			"shaders/");

		pom::GraphicsPipelineBuilder builder{};

		builder
			.SetPipelineLayout(m_PipelineLayout)
			.SetRenderPass(m_RenderPass)
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.SetShader(shaders[0], VK_SHADER_STAGE_VERTEX_BIT)
			.SetShader(shaders[1], VK_SHADER_STAGE_FRAGMENT_BIT)
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetCullMode(VK_CULL_MODE_BACK_BIT)
			.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.SetPolygonMode(VK_POLYGON_MODE_FILL)
			.SetDepthTest(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)
			.SetVertexBindingDesc(Vertex::GetBindingDescription())
			.SetVertexAttributeDesc(Vertex::GetAttributeDescriptions())
			.Build(m_Device, m_GraphicsPipeline);
		m_DeletionQueue.Push([&] { m_GraphicsPipeline.Destroy(m_Device); });


		for (auto& shader : shaders)
			shader.Destroy(m_Device);
	}

	// -- Create Frame Buffers - Requirements - [Device - SwapChain - RenderPass]
	{
		m_vFrameBuffers.clear();
		for (const pom::Image& image : m_SwapChain.GetImages())
		{
			pom::FrameBufferBuilder builder{};
			builder
				.SetRenderPass(m_RenderPass)
				.AddAttachment(image)
				.AddAttachment(m_SwapChain.GetDepthImage())
				.SetExtent(m_SwapChain.GetExtent().width, m_SwapChain.GetExtent().height)
				.Build(m_Device, m_vFrameBuffers);
		}

		m_DeletionQueueSC.Push([&] { for (auto& framebuffer : m_vFrameBuffers) framebuffer.Destroy(m_Device); m_vFrameBuffers.clear(); });
	}

	// -- Create Texture - Requirements - [Device - Allocator - Image - CommandPool]
	{
		CreateTextureImage();
	}

	// -- Create Sampler - Requirements - [Device - Physical Device]
	{
		pom::SamplerBuilder builder{};
		builder
			.SetFilters(VK_FILTER_LINEAR, VK_FILTER_LINEAR)
			.SetAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
			.EnableAnisotropy(m_PhysicalDevice.GetProperties().limits.maxSamplerAnisotropy)
			.SetMipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
			.SetMipLevels(0.f, 0.f, 0.f)
			.SetBorderColor(VK_BORDER_COLOR_INT_OPAQUE_BLACK)
			.Build(m_Device, m_TextureSampler);

		m_DeletionQueue.Push([&] { m_TextureSampler.Destroy(m_Device); });
	}

	// -- Create UBO - Requirements - [Device - Allocator - Buffer - Command Pool]
	{
		m_vUniformBuffers.resize(g_MAX_FRAMES_IN_FLIGHT);
		for (size_t i{}; i < g_MAX_FRAMES_IN_FLIGHT; ++i)
		{
			pom::BufferAllocator bufferAlloc{};
			bufferAlloc
				.SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
				.SetSize(sizeof(UniformBufferObject))
				.HostAccess(true)
				.Allocate(m_Device, m_Allocator, m_CommandPool, m_vUniformBuffers[i]);
		}
		m_DeletionQueue.Push([&] { for (auto& ubo : m_vUniformBuffers) ubo.Destroy(m_Device, m_Allocator); });
	}

	// -- Create Descriptor Pool - Requirements - [Device]
	{
		m_DescriptorPool
			.SetMaxSets(g_MAX_FRAMES_IN_FLIGHT)
			.AddPoolSizeLayout(m_DescriptorSetLayout)
			.Create(m_Device);
		m_DeletionQueue.Push([&] {m_DescriptorPool.Destroy(m_Device); });
	}

	// -- Allocate Descriptor Sets - Requirements - [Device - Descriptor Pool - Descriptor Set Layout - UBO]
	{
		m_vDescriptorSets = m_DescriptorPool.AllocateSets(m_Device, m_DescriptorSetLayout, g_MAX_FRAMES_IN_FLIGHT);

		pom::DescriptorSetWriter writer{};
		for (size_t i{}; i < g_MAX_FRAMES_IN_FLIGHT; ++i)
		{
			writer
				.WriteBuffer(m_vDescriptorSets[i], 0, m_vUniformBuffers[i], 0, sizeof(UniformBufferObject))
				.WriteImage(m_vDescriptorSets[i], 1, m_TextureImage, m_TextureSampler)
				.Execute(m_Device);
		}
	}

	// -- Create Sync Objects - Requirements - [Device]
	{
		m_SyncManager.Create(m_Device, g_MAX_FRAMES_IN_FLIGHT);
		m_DeletionQueue.Push([&] {m_SyncManager.Cleanup(); });
	}
}


//--------------------------------------------------
//    Helpers
//--------------------------------------------------
void pom::Renderer::RecreateSwapChain()
{
	auto size = m_pWindow->GetSize();
	while (size.x == 0 || size.y == 0)
	{
		size = m_pWindow->GetSize();
		glfwWaitEvents();
	}

	m_Device.WaitIdle();
	m_DeletionQueueSC.Flush();

	m_SwapChain.Recreate(m_Device, m_Allocator, m_PhysicalDevice, *m_pWindow, m_CommandPool);
	m_DeletionQueueSC.Push([&] { m_SwapChain.Destroy(m_Device, m_Allocator); });

	for (const pom::Image& image : m_SwapChain.GetImages())
	{
		pom::FrameBufferBuilder builder{};
		builder
			.SetRenderPass(m_RenderPass)
			.AddAttachment(image)
			.AddAttachment(m_SwapChain.GetDepthImage())
			.SetExtent(m_SwapChain.GetExtent().width, m_SwapChain.GetExtent().height)
			.Build(m_Device, m_vFrameBuffers);
	}
	m_DeletionQueueSC.Push([&] { for (auto& framebuffer : m_vFrameBuffers) framebuffer.Destroy(m_Device); m_vFrameBuffers.clear(); });
}
void pom::Renderer::CreateTextureImage()
{
	int texWidth;
	int texHeight;
	int texChannels;
	stbi_uc* pixels = stbi_load(g_TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
		throw std::runtime_error("Failed to load Texture Image!");

	pom::Buffer stagingBuffer;
	pom::BufferAllocator stagingAllocator{};
	stagingAllocator
		.SetUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
		.HostAccess(true)
		.SetSize(imageSize)
		.Allocate(m_Device, m_Allocator, m_CommandPool, stagingBuffer);
	vmaCopyMemoryToAllocation(m_Allocator, pixels, stagingBuffer.GetMemory(), 0, imageSize);

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
	m_DeletionQueue.Push([&] { m_TextureImage.Destroy(m_Device, m_Allocator); });

	m_CommandPool.TransitionImageLayout(m_TextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	m_CommandPool.CopyBufferToImage(stagingBuffer, m_TextureImage.GetImage(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	m_CommandPool.TransitionImageLayout(m_TextureImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	stagingBuffer.Destroy(m_Device, m_Allocator);
}
void pom::Renderer::LoadModels()
{
	// -- Load Model - Requirements - []
	{
		m_Model.LoadModel("models/viking_room.obj");
	}

	// -- Create Vertex & Index Buffer - Requirements - [Device - Allocator - Buffer - Command Pool]
	{
		m_Model.AllocateBuffers(m_Device, m_Allocator, m_CommandPool);
		m_DeletionQueue.Push([&] {m_Model.DestroyBuffers(m_Device, m_Allocator); });
	}
}
void pom::Renderer::RecordCommandBuffer(pom::CommandBuffer& commandBuffer, uint32_t imageIndex) const
{
	VkCommandBuffer vCmdBuffer = commandBuffer.GetBuffer();
	commandBuffer.Begin(0);
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass.GetRenderPass();
		renderPassInfo.framebuffer = m_vFrameBuffers[imageIndex].GetBuffer();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapChain.GetExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(vCmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			vkCmdBindPipeline(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline.GetPipeline());

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

			for (const Mesh& mesh : m_Model.meshes)
			{
				VkBuffer vertexBuffers[] = { mesh.vertexBuffer.GetBuffer() };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(vCmdBuffer, 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(vCmdBuffer, mesh.indexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

				vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout.GetLayout(), 0, 1, &m_vDescriptorSets[m_CurrentFrame].GetHandle(), 0, nullptr);

				vkCmdDrawIndexed(vCmdBuffer, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
			}
		}
		vkCmdEndRenderPass(vCmdBuffer);
	}
	commandBuffer.End();
}
void pom::Renderer::UpdateUniformBuffer(uint32_t currentImage) const
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

	vmaCopyMemoryToAllocation(m_Allocator, &ubo, m_vUniformBuffers[currentImage].GetMemory(), 0, sizeof(ubo));
}
