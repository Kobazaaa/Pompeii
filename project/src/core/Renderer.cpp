// -- Standard Library --
#include <stdexcept>
#include <array>
#include <thread>

// -- Pompeii Includes --
#include "Renderer.h"
#include "Camera.h"
#include "Debugger.h"
#include "Window.h"
#include "CommandBuffer.h"
#include "Timer.h"

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::Renderer::Renderer(Camera* pCamera, Window* pWindow)
{
	m_pWindow = pWindow;
	m_pCamera = pCamera;
	m_pScene = new SponzaScene();
	m_Context.deletionQueue.Push([&] {delete m_pScene; });
	InitializeVulkan();
}
pom::Renderer::~Renderer()
{
	m_Context.device.WaitIdle();
	m_Context.deletionQueue.Flush();
}


//--------------------------------------------------
//    Loop
//--------------------------------------------------
void pom::Renderer::Update()
{
	static bool pressX = false;
	if (!pressX && glfwGetKey(m_pWindow->GetHandle(), GLFW_KEY_X) == GLFW_PRESS)
	{
		pressX = true;
		m_pScene->AddLight(Light
			{
				/* position */	{ 7.f, 0.5f, 0.f },
				/* color */		{ 1.f, 0.651f, 0.f},
				/* lumen */		100.f, Light::Type::Point
			});
		m_Context.device.WaitIdle();
		m_LightingPass.UpdateLightDescriptors(m_Context, m_CommandPool, m_pScene);
	}
	else if (glfwGetKey(m_pWindow->GetHandle(), GLFW_KEY_X) != GLFW_PRESS)
		pressX = false;
	static bool pressC = false;
	if (!pressC && glfwGetKey(m_pWindow->GetHandle(), GLFW_KEY_C) == GLFW_PRESS)
	{
		pressC = true;
		m_pScene->PopLight();
		m_Context.device.WaitIdle();
		m_LightingPass.UpdateLightDescriptors(m_Context, m_CommandPool, m_pScene);
	}
	else if (glfwGetKey(m_pWindow->GetHandle(), GLFW_KEY_C) != GLFW_PRESS)
		pressC = false;
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
	std::jthread sceneLoader{ &Scene::Initialize, m_pScene};


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

	// -- Features --
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
	vulkan12Features.runtimeDescriptorArray = VK_TRUE;
	vulkan12Features.descriptorBindingPartiallyBound = VK_TRUE;
	vulkan12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;
	vulkan12Features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	vulkan12Features.descriptorIndexing = VK_TRUE;
	vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	vulkan12Features.pNext = &vulkan11Features;  // Chain Vulkan API 1.1 Features

	// -- Vulkan API 1.3 Features --
	VkPhysicalDeviceVulkan13Features vulkan13Features{};
	vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	vulkan13Features.dynamicRendering = VK_TRUE;
	vulkan13Features.synchronization2 = VK_TRUE;
	vulkan13Features.pNext = &vulkan12Features; // Chain Vulkan API 1.2 Features

	// -- Chaining Time --
	VkPhysicalDeviceFeatures2 features2{};
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features2.features = vulkanCoreFeatures; // Core Features
	features2.pNext = &vulkan13Features; // Chain Vulkan 1.3 features

	// -- Select GPU - Requirements - [Window - Instance]
	{
		PhysicalDeviceSelector selector;
		selector
			.AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
			.CheckForFeatures(features2)
			.PickPhysicalDevice(m_Context, m_pWindow->GetVulkanSurface());
	}

	// -- Create Device - Requirements - [Physical Device - Instance]
	{
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
			.Build(m_Context, *m_pWindow, m_SwapChain);
		m_Context.deletionQueue.Push([&] { m_SwapChain.Destroy(m_Context); });
	}

	// -- Allocate Scene - Requirements - []
	{
		sceneLoader.join();
		m_pScene->AllocateGPU(m_Context, m_CommandPool, false);
		m_Context.deletionQueue.Push([&] { m_pScene->Destroy(m_Context); });
	}

	// -- Create Descriptor Pool - Requirements - [Device]
	{
		m_DescriptorPool
			.SetDebugName("Descriptor Pool (Default)")
			.SetMaxSets(100)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
			.AddFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
			.AddFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
			.Create(m_Context);
		m_Context.deletionQueue.Push([&] {m_DescriptorPool.Destroy(m_Context); });
	}

	// -- Depth Resources --
	{
		CreateDepthResources(m_Context, m_SwapChain.GetExtent());
		m_Context.deletionQueue.Push([&] { for (const Image& image : m_vDepthImages) image.Destroy(m_Context); });
	}

	// -- Target Resources --
	{
		CreateRenderTargetResources(m_Context, m_SwapChain.GetExtent());
		m_Context.deletionQueue.Push([&] { for (const Image& image : m_vRenderTargets) image.Destroy(m_Context); });
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

	// -- Depth PrePass --
	{
		DepthPrePassCreateInfo createInfo{};
		createInfo.pDescriptorPool = &m_DescriptorPool;
		createInfo.maxFramesInFlight = m_MaxFramesInFlight;
		createInfo.depthFormat = m_vDepthImages[0].GetFormat();

		m_DepthPrePass.Initialize(m_Context, createInfo);
		m_Context.deletionQueue.Push([&] {m_DepthPrePass.Destroy(); });
	}

	// -- Forward Pass --
	{
		ForwardPassCreateInfo createInfo{};
		createInfo.pShadowPass = &m_ShadowPass;
		createInfo.pDescriptorPool = &m_DescriptorPool;
		createInfo.maxFramesInFlight = m_MaxFramesInFlight;
		createInfo.pScene = m_pScene;
		createInfo.extent = m_SwapChain.GetExtent();
		createInfo.format = m_SwapChain.GetFormat();
		createInfo.depthFormat = m_vDepthImages[0].GetFormat();

		m_ForwardPass.Initialize(m_Context, createInfo);
		m_Context.deletionQueue.Push([&] {m_ForwardPass.Destroy(); });
	}

	// -- Geometry Pass --
	{
		GeometryPassCreateInfo createInfo{};
		createInfo.pDescriptorPool = &m_DescriptorPool;
		createInfo.maxFramesInFlight = m_MaxFramesInFlight;
		createInfo.pScene = m_pScene;
		createInfo.extent = m_SwapChain.GetExtent();
		createInfo.depthFormat = m_vDepthImages[0].GetFormat();

		m_GeometryPass.Initialize(m_Context, createInfo);
		m_Context.deletionQueue.Push([&] {m_GeometryPass.Destroy(); });
	}

	// -- Lighting Pass --
	{
		LightingPassCreateInfo createInfo{};
		createInfo.pDescriptorPool = &m_DescriptorPool;
		createInfo.maxFramesInFlight = m_MaxFramesInFlight;
		createInfo.pGeometryPass = &m_GeometryPass;
		createInfo.pCommandPool = &m_CommandPool;
		createInfo.format = m_vRenderTargets.front().GetFormat();
		createInfo.pScene = m_pScene;

		m_LightingPass.Initialize(m_Context, createInfo);
		m_Context.deletionQueue.Push([&] { m_LightingPass.Destroy(); });
	}

	// -- Blit Pass --
	{
		BlitPassCreateInfo createInfo{};
		createInfo.pDescriptorPool = &m_DescriptorPool;
		createInfo.maxFramesInFlight = m_MaxFramesInFlight;
		createInfo.renderImages = &m_vRenderTargets;
		createInfo.format = m_SwapChain.GetFormat();

		m_BlitPass.Initialize(m_Context, createInfo);
		m_Context.deletionQueue.Push([&] { m_BlitPass.Destroy(); });
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

	// -- Recreate the SwapChain --
	m_SwapChain.Destroy(m_Context);
	m_SwapChain.Recreate(m_Context, *m_pWindow);

	// -- Recreate the Depth Resource --
	for (const Image& image : m_vDepthImages)
		image.Destroy(m_Context);
	CreateDepthResources(m_Context, m_SwapChain.GetExtent());

	// -- Recreate the Render Targets --
	for (const Image& image : m_vRenderTargets)
		image.Destroy(m_Context);
	CreateRenderTargetResources(m_Context, m_SwapChain.GetExtent());

	// -- Resize Passes if needed --
	m_ForwardPass.Resize(m_Context, m_SwapChain.GetExtent(), m_SwapChain.GetFormat());
	m_GeometryPass.Resize(m_Context, m_SwapChain.GetExtent());
	m_LightingPass.UpdateGBufferDescriptors(m_Context, m_GeometryPass);
	m_BlitPass.UpdateDescriptors(m_Context, m_vRenderTargets);

	// -- Update Camera Settings --
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

void pom::Renderer::CreateDepthResources(const Context& context, VkExtent2D extent)
{
	m_vDepthImages.resize(m_MaxFramesInFlight);
	for (Image& image : m_vDepthImages)
	{
		const auto format = Image::FindSupportedFormat(context.physicalDevice,
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		ImageBuilder imageBuilder{};
		imageBuilder
			.SetDebugName("Depth Buffer")
			.SetWidth(extent.width)
			.SetHeight(extent.height)
			.SetTiling(VK_IMAGE_TILING_OPTIMAL)
			//.SetSampleCount(context.physicalDevice.GetMaxSampleCount())
			.SetFormat(format)
			.SetUsageFlags(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.Build(context, image);
		image.CreateView(context, format, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);
	}
}
void pom::Renderer::CreateRenderTargetResources(const Context& context, VkExtent2D extent)
{
	m_vRenderTargets.resize(m_MaxFramesInFlight);
	for (Image& image : m_vRenderTargets)
	{
		ImageBuilder imageBuilder{};
		imageBuilder
			.SetDebugName("Render Target")
			.SetWidth(extent.width)
			.SetHeight(extent.height)
			.SetTiling(VK_IMAGE_TILING_OPTIMAL)
			//.SetSampleCount(context.physicalDevice.GetMaxSampleCount())
			.SetFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
			.SetUsageFlags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT)
			.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.Build(context, image);
		image.CreateView(context, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);
	}
}

void pom::Renderer::RecordCommandBuffer(CommandBuffer& commandBuffer, uint32_t imageIndex)
{
	Image& presentImage = m_SwapChain.GetImages()[imageIndex];
	Image& renderImage = m_vRenderTargets[imageIndex];
	Image& depthImage = m_vDepthImages[imageIndex];

	commandBuffer.Begin();
	{
		// -- Shadow Pass -- Disabled For Now
		{
			// The ShadowPass generates a depth image from the POV of the light.
			// m_ShadowPass.Record(m_Context, commandBuffer, imageIndex, m_pScene);
			// After it is done it transition the image layout.
		}

		// -- Depth Pre-Pass --
		{
			// Transition the current Depth Image to be written to
			depthImage.TransitionLayout(commandBuffer,
				VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
				VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
				VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
				0, 1, 0, 1);

			// The Depth Pre-Pass renders the entire scene to the provided depth buffer.
			m_DepthPrePass.Record(m_Context, commandBuffer, imageIndex, depthImage, m_pScene, m_pCamera);

			// Transition the current Depth Image to be read from
			depthImage.TransitionLayout(commandBuffer,
				VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
				VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
				VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
				0, 1, 0, 1);
		}

		// -- Geometry Pass --
		{
			// The Geometry Pass renders the entire scene to a GBuffer.
			m_GeometryPass.Record(m_Context, commandBuffer, imageIndex, depthImage, m_pScene, m_pCamera);
			// After it is done, the GBuffers are transitioned to a layout ready for being sampled from.
		}

		// -- Forward Pass -- Deprecated --
		{
			// The Forward Pass renders the entire screen to the provided image.
			//m_ForwardPass.Record(m_Context, commandBuffer, imageIndex, renderImage, depthImage, m_pScene, m_pCamera);
		}

		// -- Lighting Pass --
		{
			// Transition the current Render Image to be written to
			renderImage.TransitionLayout(commandBuffer,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				0, 1, 0, 1);

			// The Lighting Pass calculates all the heavy lighting calculations using the data from the Geometry Pass
			m_LightingPass.Record(m_Context, commandBuffer, imageIndex, renderImage, m_pScene, m_pCamera);

			// Transition the current Render Image to be used in the compute shader
			renderImage.TransitionLayout(commandBuffer,
				VK_IMAGE_LAYOUT_GENERAL,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
				0, 1, 0, 1);
		}

		// -- Blit Pass --
		{
			// The blit pass will blit the rendered image to the swapchain and potentially do post-processing.
			m_BlitPass.RecordCompute(m_Context, commandBuffer, imageIndex, renderImage);


			// Insert a barrier for the Render Image to be used in fragment
			renderImage.TransitionLayout(commandBuffer,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
				VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
				0, 1, 0, 1);
			// Transition the current Present Image to be written to
			presentImage.TransitionLayout(commandBuffer,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				0, 1, 0, 1);

			m_BlitPass.RecordGraphic(m_Context, commandBuffer, imageIndex, presentImage, m_pCamera);

			// At last, transition the current Present Image to be presented
			presentImage.TransitionLayout(commandBuffer,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE, 0, 1, 0, 1);
		}
	}
	commandBuffer.End();
}
