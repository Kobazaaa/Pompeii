// -- VMA --
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

// -- Standard Library --
#include <stdexcept>
#include <array>
#include <ranges>

// -- Pompeii Includes --
#include "IWindow.h"
#include "Renderer.h"
#include "RenderDebugger.h"
#include "CommandBuffer.h"
#include "RenderingItems.h"

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::Renderer::Renderer()	{ }
pompeii::Renderer::~Renderer()	{ }

void pompeii::Renderer::Initialize(IWindow* pWindow)
{
	m_pWindow = pWindow;
	InitializeVulkan();
}
void pompeii::Renderer::Deinitialize()
{
	// -- Release Resources --
	m_Context.device.WaitIdle();
	m_Context.deletionQueue.Flush();
}

//--------------------------------------------------
//    Loop
//--------------------------------------------------
bool pompeii::Renderer::StartFrame()
{
	// -- Wait for the current frame to be done --
	const auto& frameSync = m_SyncManager.GetFrameSync(m_Context.currentFrame);
	vkWaitForFences(m_Context.device.GetHandle(), 1, &frameSync.inFlight, VK_TRUE, UINT64_MAX);

	// -- Acquire new Image from SwapChain --
	VkResult result = m_SwapChain.AcquireNextImage(m_Context, frameSync.imageAvailable);

	// -- If SwapChain Image not good, recreate Swap Chain --
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		RecreateSwapChain();
		return false;
	}
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to acquire Swap Chain Image");

	// -- Reset Fence to be un-signaled (not done) --
	vkResetFences(m_Context.device.GetHandle(), 1, &frameSync.inFlight);

	for (const auto& earlyFrameExecution : m_BeforeCommandBufferExecutions)
		earlyFrameExecution();
	m_BeforeCommandBufferExecutions.clear();

	CommandBuffer& cmdBuffer = m_Context.commandPool->GetBuffer(m_Context.currentFrame);
	cmdBuffer.Reset();
	cmdBuffer.Begin();

	return true;
}
void pompeii::Renderer::RecordFrame()
{
	auto imageIndex = m_Context.currentFrame;
	CommandBuffer& commandBuffer = m_Context.commandPool->GetBuffer(imageIndex);
	Image& outputImage = m_vOutputImages[imageIndex];
	Image& renderImage = m_vRenderTargets[imageIndex];
	Image& depthImage = m_vDepthImages[imageIndex];

	// -- Shadow Pass --
	{
		m_ShadowPass.Record(m_Context, commandBuffer, m_vRenderItems, m_vLightItems);
	}

	// -- Depth Pre-Pass --
	{
		// Transition the current Depth Image to be written to
		depthImage.TransitionLayout(commandBuffer,
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
			0, depthImage.GetMipLevels(), 0, depthImage.GetLayerCount());

		// The Depth Pre-Pass renders the entire scene to the provided depth buffer.
		m_DepthPrePass.UpdateCamera(m_Context, imageIndex, m_Camera);
		m_DepthPrePass.Record(commandBuffer, m_GeometryPass, imageIndex, depthImage, m_vRenderItems);

		// Transition the current Depth Image to be read from
		depthImage.TransitionLayout(commandBuffer,
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
			0, depthImage.GetMipLevels(), 0, depthImage.GetLayerCount());
	}

	// -- Geometry Pass --
	{
		// The Geometry Pass renders the entire scene to a GBuffer.
		m_GeometryPass.UpdateCamera(m_Context, imageIndex, m_Camera);
		m_GeometryPass.Record(commandBuffer, imageIndex, depthImage, m_vRenderItems);
		// After it is done, the GBuffers are transitioned to a layout ready for being sampled from.
	}

	// -- Lighting Pass --
	{
		// Transition the current Depth Image to be sampled from
		depthImage.TransitionLayout(commandBuffer,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
			VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			0, depthImage.GetMipLevels(), 0, depthImage.GetLayerCount());

		// Transition the current Render Image to be written to
		renderImage.TransitionLayout(commandBuffer,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, renderImage.GetMipLevels(), 0, renderImage.GetLayerCount());

		// The Lighting Pass calculates all the heavy lighting calculations using the data from the Geometry Pass
		m_LightingPass.UpdateShadowMaps(m_Context, m_vLightItems);
		m_LightingPass.Record(m_Context, commandBuffer, imageIndex, renderImage, m_Camera);

		// Transition the current Render Image to be used in the compute shader
		renderImage.TransitionLayout(commandBuffer,
			VK_IMAGE_LAYOUT_GENERAL,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
			0, renderImage.GetMipLevels(), 0, renderImage.GetLayerCount());
	}

	// -- Blit Pass --
	{
		// The blit pass will blit the rendered image to the swapchain and potentially do post-processing.
		m_BlitPass.RecordCompute(commandBuffer, imageIndex, renderImage, m_Camera);

		// Insert a barrier for the Render Image to be used in fragment
		renderImage.TransitionLayout(commandBuffer,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
			VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			0, renderImage.GetMipLevels(), 0, renderImage.GetLayerCount());

		// Transition the current Present Image to be written to
		outputImage.TransitionLayout(commandBuffer,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, outputImage.GetMipLevels(), 0, outputImage.GetLayerCount());

		m_BlitPass.RecordGraphic(m_Context, commandBuffer, imageIndex, outputImage, m_Camera);
	}
}
void pompeii::Renderer::SubmitFrame()
{
	CommandBuffer& commandBuffer = m_Context.commandPool->GetBuffer(m_Context.currentFrame);
	Image& presentImage = m_SwapChain.GetCurrentImage();

	// -- Transition to Present Mode --
	presentImage.TransitionLayout(commandBuffer,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
		VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
		0, presentImage.GetMipLevels(), 0, presentImage.GetLayerCount());


	// -- End Command Buffer --
	CommandBuffer& cmdBuffer = m_Context.commandPool->GetBuffer(m_Context.currentFrame);
	cmdBuffer.End();

	// -- Get Current Info --
	const auto& frameSync = m_SyncManager.GetFrameSync(m_Context.currentFrame);

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
	uint32_t currentSwapchainIdx = m_SwapChain.GetCurrentImageIndex();
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &currentSwapchainIdx;
	presentInfo.pResults = nullptr;

	// -- Present --
	auto result = vkQueuePresentKHR(m_Context.device.GetPresentQueue(), &presentInfo);

	// -- If Present failed or out of date, recreate SwapChain --
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_pWindow->IsOutdated())
	{
		m_pWindow->ResetOutdated();
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to present Swap Chain Image!");

	for (const auto& lateFrameExecution : m_AfterCommandBufferExecutions)
		lateFrameExecution();
	m_AfterCommandBufferExecutions.clear();
}
void pompeii::Renderer::EndFrame()
{
	ClearQueue();
	m_Context.currentFrame = (m_Context.currentFrame + 1) % m_Context.maxFramesInFlight;
}

void pompeii::Renderer::ClearQueue()
{
	m_vRenderItems.clear();
	m_vLightItems.clear();
}
void pompeii::Renderer::SubmitRenderItem(const RenderItem& item)
{
	m_vRenderItems.emplace_back(item);
}
void pompeii::Renderer::SubmitLightItem(const LightItem& item)
{
	m_vLightItems.emplace_back(item);
}

void pompeii::Renderer::SetCamera(const CameraData& camera)
{
	m_Camera = camera;
}

//--------------------------------------------------
//    Accessors
//--------------------------------------------------
void pompeii::Renderer::ExecuteBeforeCommandBuffer(const std::function<void()>& func)
{
	m_BeforeCommandBufferExecutions.push_back(func);
}
void pompeii::Renderer::ExecuteAfterCommandBuffer(const std::function<void()>& func)
{
	m_AfterCommandBufferExecutions.push_back(func);
}

void pompeii::Renderer::ResizeOutput(uint32_t w, uint32_t h)
{
	if (w <= 0 || h <= 0) return;

	m_Context.device.WaitIdle();
	VkExtent2D extent = { .width = w, .height = h };

	// -- Recreate the Depth Resource --
	for (Image& image : m_vDepthImages)
		image.Destroy(m_Context);
	CreateDepthResources(m_Context, extent);

	// -- Recreate the Render Targets --
	for (Image& image : m_vRenderTargets)
		image.Destroy(m_Context);
	CreateRenderTargetResources(m_Context, extent);

	// -- Recreate the Output Targets --
	for (Image& image : m_vOutputImages)
		image.Destroy(m_Context);
	CreateOutputResources(m_Context, extent);

	// -- Resize Passes if needed --
	m_GeometryPass.Resize(m_Context, extent);
	m_LightingPass.UpdateGBufferDescriptors(m_Context, m_GeometryPass, m_vDepthImages);
	m_BlitPass.UpdateDescriptors(m_Context, m_vRenderTargets);
}
pompeii::Context& pompeii::Renderer::GetContext()					{ return m_Context; }
pompeii::Image& pompeii::Renderer::GetCurrentSwapChainImage()		{ return m_SwapChain.GetCurrentImage(); }
pompeii::Image& pompeii::Renderer::GetCurrentOutputImage()			{ return m_vOutputImages[m_Context.currentFrame]; }
std::vector<pompeii::Image>& pompeii::Renderer::GetOutputImages()	{ return m_vOutputImages; }

void pompeii::Renderer::UpdateLights(const std::vector<Light*>& lights)
{
	m_Context.device.WaitIdle();
	m_LightingPass.UpdateLightData(m_Context, lights);
}
void pompeii::Renderer::UpdateTextures(const std::vector<Image*>& textures)
{
	m_Context.device.WaitIdle();
	m_GeometryPass.UpdateTextureDescriptor(m_Context, textures);
}
void pompeii::Renderer::UpdateEnvironmentMap() const
{
	m_Context.device.WaitIdle();
	m_LightingPass.UpdateEnvironmentMap(m_Context, m_EnvMap);
}


//--------------------------------------------------
//    Vulkan Specific
//--------------------------------------------------
void pompeii::Renderer::InitializeVulkan()
{
// -- Enable Debugger - Requirements - [Debug Mode]
	{
#if _DEBUG
		RenderDebugger::SetEnabled(true);
		RenderDebugger::AddValidationLayer("VK_LAYER_KHRONOS_validation");
#else
		Debugger::SetEnabled(false);
#endif
	}

	// -- Create Instance - Requirements - []
	{
		InstanceBuilder builder;

		builder
			.SetApplicationName("Pompeii")
			.SetEngineName("PompeiiEngine")
			.SetAPIVersion(VK_API_VERSION_1_3)
			.Build(m_Context, m_pWindow);

		m_Context.deletionQueue.Push([&] { m_Context.instance.Destroy(); });
	}

	// -- Create Surface - Requirements - [Window - Instance]
	{
		m_pWindow->CreateVulkanSurface(m_Context.instance);
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
		RenderDebugger::Setup(m_Context);
		m_Context.deletionQueue.Push([&] { RenderDebugger::Destroy(); });

		RenderDebugger::SetDebugObjectName(reinterpret_cast<uint64_t>(m_Context.physicalDevice.GetHandle()),
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
		m_Context.commandPool = new CommandPool();
		m_Context.commandPool
			->Create(m_Context)
			.AllocateCmdBuffers(m_Context.maxFramesInFlight);

		m_Context.deletionQueue.Push([&] { m_Context.commandPool->Destroy(); delete m_Context.commandPool; m_Context.commandPool = nullptr; });
	}

	// -- Create SwapChain - Requirements - [Device - Allocator - Physical Device, Window, Command Pool]
	{
		VkExtent2D windowExtent = { m_pWindow->GetFramebufferSize().x, m_pWindow->GetFramebufferSize().y };
		SwapChainBuilder builder;
		builder
			.SetDesiredImageCount(m_Context.maxFramesInFlight)
			.SetImageArrayLayers(1)
			.SetImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.Build(m_Context, m_pWindow->GetVulkanSurface(), windowExtent, m_SwapChain);
		m_Context.deletionQueue.Push([&] { m_SwapChain.Destroy(m_Context); });
	}

	// -- Create Descriptor Pool - Requirements - [Device]
	{
		m_Context.descriptorPool = new DescriptorPool();
		m_Context.descriptorPool
			->SetDebugName("Descriptor Pool (Default)")
			.SetMaxSets(100)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
			.AddFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
			.AddFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
			.Create(m_Context);
		m_Context.deletionQueue.Push([&] { m_Context.descriptorPool->Destroy(m_Context); delete m_Context.descriptorPool; m_Context.descriptorPool = nullptr; });
	}

	// -- Depth Resources --
	{
		CreateDepthResources(m_Context, m_SwapChain.GetExtent());
		m_Context.deletionQueue.Push([&] { for (Image& image : m_vDepthImages) image.Destroy(m_Context); });
	}

	// -- Target Resources --
	{
		CreateRenderTargetResources(m_Context, m_SwapChain.GetExtent());
		m_Context.deletionQueue.Push([&] { for (Image& image : m_vRenderTargets) image.Destroy(m_Context); });
	}

	// -- Output Resources --
	{
		CreateOutputResources(m_Context, m_SwapChain.GetExtent());
		m_Context.deletionQueue.Push([&] { for (Image& image : m_vOutputImages) image.Destroy(m_Context); });
	}

	// -- Geometry Pass --
	{
		GeometryPassCreateInfo createInfo{};
		createInfo.extent = m_SwapChain.GetExtent();
		createInfo.depthFormat = m_vDepthImages[0].GetFormat();

		m_GeometryPass.Initialize(m_Context, createInfo);
		m_Context.deletionQueue.Push([&] {m_GeometryPass.Destroy(); });
	}

	// -- Shadow Pass --
	{
		m_ShadowPass.Initialize(m_Context);
		m_Context.deletionQueue.Push([&] {m_ShadowPass.Destroy(); });
	}

	// -- Depth PrePass --
	{
		DepthPrePassCreateInfo createInfo{};
		createInfo.depthFormat = m_vDepthImages[0].GetFormat();
		createInfo.pGeometryPass = &m_GeometryPass;

		m_DepthPrePass.Initialize(m_Context, createInfo);
		m_Context.deletionQueue.Push([&] {m_DepthPrePass.Destroy(); });
	}

	// -- Lighting Pass --
	{
		LightingPassCreateInfo createInfo{};
		createInfo.pGeometryPass = &m_GeometryPass;
		createInfo.format = m_vRenderTargets.front().GetFormat();
		createInfo.pDepthImages = &m_vDepthImages;

		m_LightingPass.Initialize(m_Context, createInfo);
		m_Context.deletionQueue.Push([&] { m_LightingPass.Destroy(); });

		//todo no
		m_EnvMap
			.CreateSampler(m_Context)
			.CreateSkyboxCube(m_Context, "textures/golden_gate_hills_4k.hdr")
			.CreateDiffIrradianceMap(m_Context)
			.CreateSpecIrradianceMap(m_Context)
			.CreateBRDFLut(m_Context);
		m_LightingPass.UpdateEnvironmentMap(m_Context, m_EnvMap);
		m_Context.deletionQueue.Push([&] { m_EnvMap.Destroy(m_Context); });
	}

	// -- Blit Pass --
	{
		BlitPassCreateInfo createInfo{};
		createInfo.renderImages = &m_vRenderTargets;
		createInfo.format = m_vOutputImages.front().GetFormat();

		m_BlitPass.Initialize(m_Context, createInfo);
		m_Context.deletionQueue.Push([&] { m_BlitPass.Destroy(); });
	}

	// -- Create Sync Objects - Requirements - [Device]
	{
		m_SyncManager.Create(m_Context, m_Context.maxFramesInFlight);
		m_Context.deletionQueue.Push([&] {m_SyncManager.Cleanup(m_Context); });
	}
}

//--------------------------------------------------
//    Helpers
//--------------------------------------------------
void pompeii::Renderer::RecreateSwapChain()
{
	auto size = m_pWindow->GetFramebufferSize();
	while (size.x == 0 || size.y == 0)
	{
		size = m_pWindow->GetFramebufferSize();
		m_pWindow->PollEvents();
	}

	m_Context.device.WaitIdle();

	// -- Recreate the SwapChain --
	VkExtent2D windowExtent = { m_pWindow->GetFramebufferSize().x, m_pWindow->GetFramebufferSize().y };
	m_SwapChain.Destroy(m_Context);
	m_SwapChain.Recreate(m_Context, m_pWindow->GetVulkanSurface(), windowExtent);

	// -- Recreate the Depth Resource --
	//for (Image& image : m_vDepthImages)
	//	image.Destroy(m_Context);
	//CreateDepthResources(m_Context, m_SwapChain.GetExtent());

	// -- Recreate the Render Targets --
	//for (Image& image : m_vRenderTargets)
	//	image.Destroy(m_Context);
	//CreateRenderTargetResources(m_Context, m_SwapChain.GetExtent());

	// -- Recreate the Output Targets --
	//for (Image& image : m_vOutputImages)
	//	image.Destroy(m_Context);
	//CreateOutputResources(m_Context, m_SwapChain.GetExtent());

	// -- Resize Passes if needed --
	//m_GeometryPass.Resize(m_Context, m_SwapChain.GetExtent());
	//m_LightingPass.UpdateGBufferDescriptors(m_Context, m_GeometryPass, m_vDepthImages);
	//m_BlitPass.UpdateDescriptors(m_Context, m_vRenderTargets);

	// -- Update Camera Settings --
	//todo add event to be able to hook into on resize, and update camera settings here!
	//const CameraSettings& oldSettings = camera->GetSettings();
	//const CameraSettings settings
	//{
	//	.fov			= oldSettings.fov,
	//	.aspectRatio	= m_pWindow->GetAspectRatio(),
	//	.nearPlane		= oldSettings.nearPlane,
	//	.farPlane		= oldSettings.farPlane
	//};
	//camera->ChangeSettings(settings);
}

void pompeii::Renderer::CreateDepthResources(const Context& context, VkExtent2D extent)
{
	m_vDepthImages.resize(m_Context.maxFramesInFlight);
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
			.SetUsageFlags(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.Build(context, image);
		image.CreateView(context, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);
	}
}
void pompeii::Renderer::CreateRenderTargetResources(const Context& context, VkExtent2D extent)
{
	m_vRenderTargets.resize(context.maxFramesInFlight);
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
		image.CreateView(context, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);
	}
}
void pompeii::Renderer::CreateOutputResources(const Context& context, VkExtent2D extent)
{
	m_vOutputImages.resize(context.maxFramesInFlight);
	for (Image& image : m_vOutputImages)
	{
		ImageBuilder imageBuilder{};
		imageBuilder
			.SetDebugName("Output Image")
			.SetWidth(extent.width)
			.SetHeight(extent.height)
			.SetTiling(VK_IMAGE_TILING_OPTIMAL)
			.SetFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
			.SetUsageFlags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.Build(context, image);
		image.CreateView(context, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);
	}
}
