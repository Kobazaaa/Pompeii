// -- Standard Library --
#include <stdexcept>
#include <array>

// -- Pompeii Includes --
#include "Renderer.h"
#include "Camera.h"
#include "Debugger.h"
#include "Window.h"
#include "Shader.h"
#include "CommandBuffer.h"

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::Renderer::Initialize(Camera* pCamera, Window* pWindow)
{
	m_pWindow = pWindow;
	m_pCamera = pCamera;
	InitializeVulkan();
}

void pom::Renderer::Destroy()
{
	m_Context.device.WaitIdle();
	m_DeletionQueueSC.Flush();
	m_Context.deletionQueue.Flush();
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
	vkWaitForFences(m_Context.device.GetHandle(), 1, &frameSync.inFlight, VK_TRUE, UINT64_MAX);

	// -- Acquire new Image from SwapChain --
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_Context.device.GetHandle(), m_SwapChain.GetHandle(), UINT64_MAX, frameSync.imageAvailable, VK_NULL_HANDLE, &imageIndex);

	// -- If SwapChain Image not good, recreate Swap Chain --
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		RecreateSwapChain();
		return;
	}
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to acquire Swap Chain Image");

	// -- Reset Fence to be un-signaled (not done) --
	vkResetFences(m_Context.device.GetHandle(), 1, &frameSync.inFlight);

	// -- Record Command Buffer --
	CommandBuffer& cmdBuffer = m_CommandPool.GetBuffer(m_CurrentFrame);
	cmdBuffer.Reset();
	RecordCommandBuffer(cmdBuffer, imageIndex);

	// -- Submit Commands with Semaphores --
	const SemaphoreInfo semaphoreInfo
	{
		.vWaitSemaphores = { frameSync.imageAvailable },
		.vWaitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },
		.vSignalSemaphores = { frameSync.renderFinished }
	};
	cmdBuffer.Submit(m_Context.device.GetGraphicQueue(), false, semaphoreInfo, frameSync.inFlight);

	// -- Create Present Info --
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = static_cast<uint32_t>(semaphoreInfo.vSignalSemaphores.size());
	presentInfo.pWaitSemaphores = semaphoreInfo.vSignalSemaphores.data();

	VkSwapchainKHR swapChains[] = { m_SwapChain.GetHandle() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	// -- Present --
	result = vkQueuePresentKHR(m_Context.device.GetPresentQueue(), &presentInfo);

	// -- If Present failed or out of date, recreate SwapChain --
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_pWindow->IsOutdated())
	{
		m_pWindow->ResetOutdated();
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to present Swap Chain Image!");

	// -- Go to next frame --
	m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFramesInFlight;
}


//--------------------------------------------------
//    Vulkan Specific
//--------------------------------------------------
void pom::Renderer::InitializeVulkan()
{
	// -- Enable Debugger - Requirements - [Debug Mode]
	{
#if _DEBUG
		Debugger::SetEnabled(true);
		Debugger::AddValidationLayer("VK_LAYER_KHRONOS_validation");
#else
		pom::Debugger::SetEnabled(false);
#endif
	}

	// -- Create Instance - Requirements - []
	{
		InstanceBuilder builder;

		builder
			.SetApplicationName("Pompeii")
			.SetEngineName("PompeiiEngine")
			.SetAPIVersion(VK_API_VERSION_1_3)
			.Build(m_Context);

		m_Context.deletionQueue.Push([&] { m_Context.instance.Destroy(); });
	}

	// -- Create Surface - Requirements - [Window - Instance]
	{
		m_pWindow->CreateVulkanSurface(m_Context);
		m_Context.deletionQueue.Push([&] { vkDestroySurfaceKHR(m_Context.instance.GetHandle(), m_pWindow->GetVulkanSurface(), nullptr); });
	}

	// -- Select GPU - Requirements - [Window - Instance]
	{
		PhysicalDeviceSelector selector;
		selector
			.AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
			.PickPhysicalDevice(m_Context, m_pWindow->GetVulkanSurface());
	}

	// -- Create Device - Requirements - [Physical Device - Instance]
	{
		// -- Vulkan API Core Features --
		VkPhysicalDeviceFeatures vulkanCoreFeatures{};
		vulkanCoreFeatures.samplerAnisotropy = VK_TRUE;
		vulkanCoreFeatures.fillModeNonSolid = VK_TRUE;
		vulkanCoreFeatures.sampleRateShading = VK_TRUE;

		// -- Vulkan API 1.1 Features --
		VkPhysicalDeviceVulkan11Features vulkan11Features{};
		vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
		vulkan11Features.pNext = nullptr;  // End of Chain

		// -- Vulkan API 1.2 Features --
		VkPhysicalDeviceVulkan12Features vulkan12Features{};
		vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		vulkan12Features.pNext = &vulkan11Features;  // Chain Vulkan API 1.1 Features

		// -- Vulkan API 1.3 Features --
		VkPhysicalDeviceVulkan13Features vulkan13Features{};
		vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		vulkan13Features.synchronization2 = VK_TRUE;
		vulkan13Features.pNext = &vulkan12Features; // Chain Vulkan API 1.2 Features

		// -- Chaining Time --
		VkPhysicalDeviceFeatures2 features2{};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.features = vulkanCoreFeatures; // Core Features
		features2.pNext = &vulkan13Features; // Chain Vulkan 1.3 features

		DeviceBuilder deviceBuilder{};

		deviceBuilder
			.SetFeatures(features2)
			.Build(m_Context);

		m_Context.deletionQueue.Push([&] { m_Context.device.Destroy(); });
	}

	// -- Setup Debugger - Requirements - [Instance]
	{
#if _DEBUG
		Debugger::Setup(m_Context);
		m_Context.deletionQueue.Push([&] { Debugger::Destroy(); });

		Debugger::SetDebugObjectName(reinterpret_cast<uint64_t>(m_Context.physicalDevice.GetHandle()),
			VK_OBJECT_TYPE_PHYSICAL_DEVICE, m_Context.physicalDevice.GetProperties().deviceName);
#endif
	}

	// -- Create Allocator - Requirements - [Device - Physical Device - Instance]
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = m_Context.physicalDevice.GetHandle();
		allocatorInfo.device = m_Context.device.GetHandle();
		allocatorInfo.instance = m_Context.instance.GetHandle();

		vmaCreateAllocator(&allocatorInfo, &m_Context.allocator);
		m_Context.deletionQueue.Push([&] { vmaDestroyAllocator(m_Context.allocator); });
	}

	// -- Create Command Pool - Requirements - [Device - Physical Device]
	{
		m_CommandPool
			.Create(m_Context)
			.AllocateCmdBuffers(m_MaxFramesInFlight);

		m_Context.deletionQueue.Push([&] { m_CommandPool.Destroy(); });
	}

	// -- Create SwapChain - Requirements - [Device - Allocator - Physical Device, Window, Command Pool]
	{
		SwapChainBuilder builder;

		builder
			.SetDesiredImageCount(m_MaxFramesInFlight)
			.SetImageArrayLayers(1)
			.SetImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			.Build(m_Context, *m_pWindow, m_SwapChain, m_CommandPool);

		m_DeletionQueueSC.Push([&] { m_SwapChain.Destroy(m_Context); });
	}

	// -- Load Models - Requirements - []
	{
		LoadModels();
	}

	// -- Create Render Pass - Requirements - [Device - SwapChain - Depth Buffer]
	{
		RenderPassBuilder builder{};

		builder
			.NewAttachment()
				.SetFormat(m_SwapChain.GetFormat())
				.SetSamples(m_Context.physicalDevice.GetMaxSampleCount())
				.SetLoadStoreOp(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
				.SetStencilLoadStoreOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE)
				.SetInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
				.SetFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
				.AddColorAttachment(0)
			.NewAttachment()
				.SetFormat(m_SwapChain.GetDepthImage().GetFormat())
				.SetSamples(m_Context.physicalDevice.GetMaxSampleCount())
				.SetLoadStoreOp(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE)
				.SetStencilLoadStoreOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE)
				.SetInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
				.SetFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
				.AddDepthAttachment(1)
			.NewAttachment()
				.SetFormat(m_SwapChain.GetFormat())
				.SetSamples(VK_SAMPLE_COUNT_1_BIT)
				.SetLoadStoreOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE)
				.SetStencilLoadStoreOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE)
				.SetInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
				.SetFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
				.AddResolveAttachment(2)
			.NewSubpass()
				.SetBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS)
			.NewDependency()
				.SetSrcSubPass(VK_SUBPASS_EXTERNAL)
				.SetDstSubPass(0)
				.SetSrcMasks(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0)
				.SetDstMasks(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
			.Build(m_Context, m_RenderPass);
		m_Context.deletionQueue.Push([&] { m_RenderPass.Destroy(m_Context); });
	}

	// -- Create Descriptor Set Layout - Requirements - [Device]
	{
		DescriptorSetLayoutBuilder builder{};

		// -- Uniform Buffer Descriptor --
		builder
			.SetDebugName("Uniform Buffer DS Layout")
			.NewLayoutBinding()
				.SetType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				.SetShaderStages(VK_SHADER_STAGE_VERTEX_BIT)
			.Build(m_Context, m_UniformDSL);
		m_Context.deletionQueue.Push([&] { m_UniformDSL.Destroy(m_Context); });

		// -- Texture Array Descriptor --
		builder
			.SetDebugName("Texture Array DS Layout")
			.NewLayoutBinding()
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
				.SetCount(static_cast<uint32_t>(m_Model.images.size()))
			.Build(m_Context, m_TextureDSL);
		m_Context.deletionQueue.Push([&] { m_TextureDSL.Destroy(m_Context); });
	}

	// -- Create Graphics Pipeline Layout - Requirements - [Device - Descriptor Set Layout]
	{
		GraphicsPipelineLayoutBuilder builder{};

		builder
			.NewPushConstantRange()
				.SetPCOffset(0)
				.SetPCSize(sizeof(MeshPushConstants))
				.SetPCStageFlags(VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddLayout(m_UniformDSL)
			.AddLayout(m_TextureDSL)
			.Build(m_Context, m_PipelineLayout);
		m_Context.deletionQueue.Push([&] {m_PipelineLayout.Destroy(m_Context); });
	}

	// -- Create Graphics Pipeline - Requirements - [Pipeline Layout - Shaders - RenderPass]
	{
		ShaderLoader shaderLoader{};

		ShaderModule vertShader;
		ShaderModule fragShader;
		shaderLoader.Load(m_Context, "shaders/shader.vert.spv", vertShader);
		shaderLoader.Load(m_Context, "shaders/shader.frag.spv", fragShader);

		GraphicsPipelineBuilder builder{};
		uint32_t arraySize = static_cast<uint32_t>(m_Model.images.size());
		builder
			.SetDebugName("Graphics Pipeline (Default)")
			.SetPipelineLayout(m_PipelineLayout)
			.SetRenderPass(m_RenderPass)
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
			.SetSampleCount(m_Context.physicalDevice.GetMaxSampleCount())
			.SetDepthTest(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)
			.SetVertexBindingDesc(Vertex::GetBindingDescription())
			.SetVertexAttributeDesc(Vertex::GetAttributeDescriptions())
			.Build(m_Context, m_GraphicsPipeline);
		m_Context.deletionQueue.Push([&] { m_GraphicsPipeline.Destroy(m_Context); });

		builder = {};

		builder
			.SetDebugName("Graphics Pipeline (Transparent)")
			.SetPipelineLayout(m_PipelineLayout)
			.SetRenderPass(m_RenderPass)
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.AddShader(vertShader, VK_SHADER_STAGE_VERTEX_BIT)
			.AddShader(fragShader, VK_SHADER_STAGE_FRAGMENT_BIT)
				.SetShaderSpecialization(0, 0, sizeof(uint32_t), &arraySize)
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetCullMode(VK_CULL_MODE_NONE)
			.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.SetPolygonMode(VK_POLYGON_MODE_FILL)
			.SetSampleCount(m_Context.physicalDevice.GetMaxSampleCount())
			.SetDepthTest(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)
			.SetVertexBindingDesc(Vertex::GetBindingDescription())
			.SetVertexAttributeDesc(Vertex::GetAttributeDescriptions())
			.EnableBlend()
				.SetColorBlend(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD)
				.SetAlphaBlend(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD)
			.Build(m_Context, m_TransPipeline);
		m_Context.deletionQueue.Push([&] { m_TransPipeline.Destroy(m_Context); });

		fragShader.Destroy(m_Context);
		vertShader.Destroy(m_Context);
	}

	// -- Create Frame Buffers & MSAA Image - Requirements - [Device - SwapChain - RenderPass]
	{
		CreateFrameBuffers();
	}

	// -- Create Sampler - Requirements - [Device - Physical Device]
	{
		SamplerBuilder builder{};
		builder
			.SetFilters(VK_FILTER_LINEAR, VK_FILTER_LINEAR)
			.SetAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
			.EnableAnisotropy(m_Context.physicalDevice.GetProperties().limits.maxSamplerAnisotropy)
			.SetMipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
			.SetMipLevels(0.f, 0.f, VK_LOD_CLAMP_NONE)
			.SetBorderColor(VK_BORDER_COLOR_INT_OPAQUE_BLACK)
			.Build(m_Context, m_TextureSampler);

		m_Context.deletionQueue.Push([&] { m_TextureSampler.Destroy(m_Context); });
	}

	// -- Create UBO - Requirements - [Device - Allocator - Buffer - Command Pool]
	{
		m_vUniformBuffers.resize(m_MaxFramesInFlight);
		for (size_t i{}; i < m_MaxFramesInFlight; ++i)
		{
			BufferAllocator bufferAlloc{};
			bufferAlloc
				.SetDebugName("Uniform Buffer (Matrices)")
				.SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
				.SetSize(sizeof(UniformBufferObject))
				.HostAccess(true)
				.Allocate(m_Context, m_vUniformBuffers[i]);
		}
		m_Context.deletionQueue.Push([&] { for (auto& ubo : m_vUniformBuffers) ubo.Destroy(m_Context); });
	}

	// -- Create Descriptor Pool - Requirements - [Device]
	{
		m_DescriptorPool
			.SetDebugName("Descriptor Pool (Default)")
			.SetMaxSets(30)
			.AddPoolSizeLayout(m_UniformDSL)
			.AddPoolSizeLayout(m_TextureDSL)
			.Create(m_Context);
		m_Context.deletionQueue.Push([&] {m_DescriptorPool.Destroy(m_Context); });
	}

	// -- Allocate Descriptor Sets - Requirements - [Device - Descriptor Pool - Descriptor Set Layout - UBO]
	{
		m_vUniformDS = m_DescriptorPool.AllocateSets(m_Context, m_UniformDSL, m_MaxFramesInFlight, "Uniform Buffer DS");
		m_vTextureDS = m_DescriptorPool.AllocateSets(m_Context, m_TextureDSL, 1, "Texture Array DS");

		// -- Write UBO --
		DescriptorSetWriter writer{};
		for (size_t i{}; i < m_MaxFramesInFlight; ++i)
		{
			writer
				.AddBufferInfo(m_vUniformBuffers[i], 0, sizeof(UniformBufferObject))
				.WriteBuffers(m_vUniformDS[i], 0)
				.Execute(m_Context);
		}

		// -- Write Textures --
		for (Image& image : m_Model.images)
		{
			writer.AddImageInfo(image, m_TextureSampler);
		}
		writer.WriteImages(m_vTextureDS[0], 0).Execute(m_Context);
	}

	// -- Create Sync Objects - Requirements - [Device]
	{
		m_SyncManager.Create(m_Context, m_MaxFramesInFlight);
		m_Context.deletionQueue.Push([&] {m_SyncManager.Cleanup(m_Context); });
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
		Window::WaitEvents();
	}

	m_Context.device.WaitIdle();
	m_DeletionQueueSC.Flush();

	m_SwapChain.Recreate(m_Context, *m_pWindow, m_CommandPool);
	m_DeletionQueueSC.Push([&] { m_SwapChain.Destroy(m_Context); });

	CreateFrameBuffers();

	const CameraSettings& oldSettings = m_pCamera->GetSettings();
	const CameraSettings settings
	{
		.fov			= oldSettings.fov,
		.aspectRatio	= m_pWindow->GetAspectRatio(),
		.nearPlane		= oldSettings.nearPlane,
		.farPlane		= oldSettings.farPlane
	};
	m_pCamera->ChangeSettings(settings);
}
void pom::Renderer::CreateFrameBuffers()
{
	ImageBuilder iBuilder{};
	iBuilder
		.SetDebugName("MSAA Buffer")
		.SetWidth(m_SwapChain.GetExtent().width)
		.SetHeight(m_SwapChain.GetExtent().height)
		.SetFormat(m_SwapChain.GetFormat())
		.SetMipLevels(1)
		.SetSampleCount(m_Context.physicalDevice.GetMaxSampleCount())
		.SetTiling(VK_IMAGE_TILING_OPTIMAL)
		.SetUsageFlags(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		.Build(m_Context, m_MSAAImage);
	m_MSAAImage.CreateView(m_Context, m_SwapChain.GetFormat(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);
	m_DeletionQueueSC.Push([&] { m_MSAAImage.Destroy(m_Context); });

	for (const VkImageView& view : m_SwapChain.GetViewHandles())
	{
		FrameBufferBuilder builder{};
		builder
			.SetRenderPass(m_RenderPass)
			.AddAttachment(m_MSAAImage.GetViewHandle())
			.AddAttachment(m_SwapChain.GetDepthImage().GetViewHandle())
			.AddAttachment(view)
			.SetExtent(m_SwapChain.GetExtent().width, m_SwapChain.GetExtent().height)
			.Build(m_Context, m_vFrameBuffers);
	}
	m_DeletionQueueSC.Push([&] { for (auto& framebuffer : m_vFrameBuffers) framebuffer.Destroy(m_Context); m_vFrameBuffers.clear(); });
}

void pom::Renderer::LoadModels()
{
	// -- Load Model - Requirements - []
	{
		m_Model.LoadModel("models/sponza.obj");
	}

	// -- Create Vertex & Index Buffer - Requirements - [Device - Allocator - Buffer - Command Pool]
	{
		m_Model.AllocateResources(m_Context, m_CommandPool, false);
		m_Context.deletionQueue.Push([&] {m_Model.Destroy(m_Context); });
	}
}
void pom::Renderer::RecordCommandBuffer(CommandBuffer& commandBuffer, uint32_t imageIndex) const
{
	const VkCommandBuffer& vCmdBuffer = commandBuffer.GetHandle();
	commandBuffer.Begin(0);
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass.GetHandle();
		renderPassInfo.framebuffer = m_vFrameBuffers[imageIndex].GetHandle();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapChain.GetExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.53f, 0.81f, 0.92f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		Debugger::BeginDebugLabel(commandBuffer, "Render Pass", glm::vec4(0.6f, 0.2f, 0.8f, 1));
		vkCmdBeginRenderPass(vCmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			// -- Set Dynamic Viewport --
			VkViewport viewport;
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(m_SwapChain.GetExtent().width);
			viewport.height = static_cast<float>(m_SwapChain.GetExtent().height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(vCmdBuffer, 0, 1, &viewport);
			Debugger::InsertDebugLabel(commandBuffer, "Bind Viewport", glm::vec4(0.2f, 1.f, 0.2f, 1.f));

			// -- Set Dynamic Scissors --
			VkRect2D scissor;
			scissor.offset = { .x = 0, .y = 0};
			scissor.extent = m_SwapChain.GetExtent();
			vkCmdSetScissor(vCmdBuffer, 0, 1, &scissor);
			Debugger::InsertDebugLabel(commandBuffer, "Bind Scissor", glm::vec4(1.f, 1.f, 0.2f, 1.f));

			// -- Bind Descriptor Sets --
			vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout.GetHandle(), 0, 1, &m_vUniformDS[m_CurrentFrame].GetHandle(), 0, nullptr);
			Debugger::InsertDebugLabel(commandBuffer, "Bind Uniform Buffer", glm::vec4(0.f, 1.f, 1.f, 1.f));
			vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout.GetHandle(), 1, 1, &m_vTextureDS[0].GetHandle(), 0, nullptr);
			Debugger::InsertDebugLabel(commandBuffer, "Bind Textures", glm::vec4(0.f, 1.f, 1.f, 1.f));

			// -- Bind Model Data --
			m_Model.Bind(commandBuffer);

			// -- Draw Opaque --
			vkCmdBindPipeline(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline.GetHandle());
			Debugger::InsertDebugLabel(commandBuffer, "Bind Pipeline (Default)", glm::vec4(0.2f, 0.4f, 1.f, 1.f));
			m_Model.DrawOpaque(commandBuffer, m_PipelineLayout);

			// -- Draw Transparent --
			vkCmdBindPipeline(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_TransPipeline.GetHandle());
			Debugger::InsertDebugLabel(commandBuffer, "Bind Pipeline (Transparent)", glm::vec4(0.2f, 0.4f, 1.f, 1.f));
			m_Model.DrawTransparent(commandBuffer, m_PipelineLayout);

		}
		vkCmdEndRenderPass(vCmdBuffer);
		Debugger::EndDebugLabel(commandBuffer);
	}
	commandBuffer.End();
}
void pom::Renderer::UpdateUniformBuffer(uint32_t currentImage) const
{
	UniformBufferObject ubo;
	//ubo.model = glm::scale(glm::rotate(glm::mat4(1.0f), Timer::GetTotalTime() * glm::radians(90.f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(0.25, 0.25, 0.25)) ;
	ubo.model = glm::mat4(1.f);
	ubo.view = m_pCamera->GetViewMatrix();
	ubo.proj = m_pCamera->GetProjectionMatrix();

	vmaCopyMemoryToAllocation(m_Context.allocator, &ubo, m_vUniformBuffers[currentImage].GetMemoryHandle(), 0, sizeof(ubo));
}
