// -- Standard Library --
#include <stdexcept>
#include <array>
#include <thread>

// -- Pompeii Includes --
#include "Renderer.h"
#include "Camera.h"
#include "Debugger.h"
#include "Window.h"
#include "Shader.h"
#include "CommandBuffer.h"
#include "Timer.h"

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::Renderer::Initialize(Camera* pCamera, Window* pWindow)
{
	m_pWindow = pWindow;
	m_pCamera = pCamera;
	m_pScene = new Scene();
	m_Context.deletionQueue.Push([&] {delete m_pScene; });
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
	// -- Start Loading Scene on CPU --
	std::thread sceneLoader{ &Scene::Load, m_pScene, "models/sponza.obj" };


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
		//todo check on PhysicalDevice if features are available!
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

	// -- Allocate Scene - Requirements - []
	{
		sceneLoader.join();
		m_pScene->Allocate(m_Context, m_CommandPool, false);
		m_Context.deletionQueue.Push([&] { m_pScene->Destroy(); });
	}

	// -- Create Descriptor Pool - Requirements - [Device]
	{
		m_DescriptorPool
			.SetDebugName("Descriptor Pool (Default)")
			.SetMaxSets(100)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
			.Create(m_Context);
		m_Context.deletionQueue.Push([&] {m_DescriptorPool.Destroy(m_Context); });
	}

	// -- Shadow Pass --
	{
		ShadowPassCreateInfo createInfo{};
		createInfo.extent = glm::vec2(8192, 8192);
		createInfo.pDescriptorPool = &m_DescriptorPool;
		createInfo.maxFramesInFlight = m_MaxFramesInFlight;

		m_ShadowPass.Initialize(m_Context, createInfo);
		m_Context.deletionQueue.Push([&] {m_ShadowPass.Destroy(); });
	}

	// -- Forward Pass --
	{
		ForwardPassCreateInfo createInfo{};
		createInfo.pShadowPass = &m_ShadowPass;
		createInfo.pDescriptorPool = &m_DescriptorPool;
		createInfo.maxFramesInFlight = m_MaxFramesInFlight;
		createInfo.pScene = m_pScene;
		createInfo.pSwapChain = &m_SwapChain;

		m_ForwardPass.Initialize(m_Context, createInfo);
		m_Context.deletionQueue.Push([&] {m_ForwardPass.Destroy(); });
	}

	// -- Create Frame Buffers & MSAA Image - Requirements - [Device - SwapChain - RenderPass]
	{
		CreateFrameBuffers();
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
			.SetRenderPass(m_ForwardPass.GetRenderPass())
			.AddAttachment(m_MSAAImage.GetViewHandle())
			.AddAttachment(m_SwapChain.GetDepthImage().GetViewHandle())
			.AddAttachment(view)
			.SetExtent(m_SwapChain.GetExtent().width, m_SwapChain.GetExtent().height)
			.Build(m_Context, m_vFrameBuffers);
	}
	m_DeletionQueueSC.Push([&] { for (auto& framebuffer : m_vFrameBuffers) framebuffer.Destroy(m_Context); m_vFrameBuffers.clear(); });
}

void pom::Renderer::RecordCommandBuffer(CommandBuffer& commandBuffer, uint32_t imageIndex)
{
	commandBuffer.Begin();
	{
		// -- Record Shadow Pass --
		m_ShadowPass.Record(m_Context, commandBuffer, imageIndex, m_pScene);

		// -- Sync --
		// No explicit synchronization needed, as the subpass dependencies of the Shadow Pass take care of this
		// -- Sync --

		// -- Record Forward Pass --
		m_ForwardPass.Record(m_Context, m_vFrameBuffers[imageIndex], m_SwapChain, commandBuffer, imageIndex, m_pScene, m_pCamera);
	}
	commandBuffer.End();
}
