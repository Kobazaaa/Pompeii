// -- Pompeii Includes --
#include "VulkanRenderer.h"
#include "IWindow.h"
#include "RenderDebugger.h"
#include "FrameContext.h"

// -- Standard Library --
#include <stdexcept>

#include "VulkanCommandBuffer.h"
#include "../../../../../../../out/build/x64-Debug/_deps/glfw-src/include/GLFW/glfw3.h"

using namespace pompeii;
using namespace pompeii::vulkan;

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
VulkanRenderer::VulkanRenderer(IWindow* pWindow)
	: m_pWindow(pWindow)
{
	InitializeVulkan();
}

VulkanRenderer::~VulkanRenderer()
{
	Shutdown();
}


//--------------------------------------------------
//    Functionality
//--------------------------------------------------
void VulkanRenderer::AddRenderPass(std::unique_ptr<IRenderPass> pass)
{
	m_vRenderPasses.push_back(std::move(pass));
	m_vRenderPasses.back()->Setup(this);
}

void VulkanRenderer::BeginFrame()
{
	FrameContext& frame = m_vFrames[m_CurrentFrame];
	if (!m_Context.swapchain->AcquireNextImage(frame))
		return;
}

void VulkanRenderer::Execute()
{
	// Record frame
	FrameContext& frame = m_vFrames[m_CurrentFrame];
	const auto& cmd = frame.CommandBuffer.get();
	cmd->Begin();
	for (const auto& pass : m_vRenderPasses)
	{
		pass->Execute(cmd);
	}
}

void VulkanRenderer::EndFrame()
{
	FrameContext& frame = m_vFrames[m_CurrentFrame];
	const auto& cmd = frame.CommandBuffer.get();
	cmd->End();
	Submit();

	m_Context.swapchain->Present(frame);
	m_CurrentFrame = (m_CurrentFrame + 1) % m_vFrames.size();
}

void VulkanRenderer::Shutdown()
{
	m_Context.device.WaitIdle();
}


//--------------------------------------------------
//    Helpers
//--------------------------------------------------
void VulkanRenderer::InitializeVulkan()
{
	RenderDebugger::SetEnabled(true);
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

	CreateInstance();
	CreateSurface();
	ChoosePhysicalDevice(features2);
	CreateDevice(features2);
	CreateAllocator();
	CreateFrameContext();
	CreateCommandPool();
	CreateSwapChain();
}

void VulkanRenderer::CreateInstance()
{
	InstanceBuilder builder{};
	builder
		.SetApplicationName("Pompeii")
		.SetEngineName("PompeiiEngine")
		.SetAPIVersion(VK_API_VERSION_1_3);

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	for (uint32_t index{}; index < glfwExtensionCount; ++index)
		builder.AddInstanceExtension(glfwExtensions[index]);

	builder.Build(m_Context);
}
void VulkanRenderer::CreateSurface()
{
	// todo wtaf is this
	if (glfwCreateWindowSurface(m_Context.instance, static_cast<GLFWwindow*>(m_pWindow->GetNativeHandle()), nullptr, &m_Context.surface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Window Surface!");
}
void VulkanRenderer::CreateDevice(VkPhysicalDeviceFeatures2 features)
{
	DeviceBuilder builder{};
	builder
		.SetFeatures(features)
		.Build(m_Context);
}
void VulkanRenderer::ChoosePhysicalDevice(VkPhysicalDeviceFeatures2 features)
{
	PhysicalDeviceSelector selector;
	selector
		.AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
		.CheckForFeatures(features)
		.PickPhysicalDevice(m_Context);
}
void VulkanRenderer::CreateAllocator()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = m_Context.physicalDevice;
	allocatorInfo.device = m_Context.device;
	allocatorInfo.instance = m_Context.instance;
	vmaCreateAllocator(&allocatorInfo, &m_Context.allocator);
}
void VulkanRenderer::CreateFrameContext()
{
	m_vFrames.clear();
	m_vFrames.reserve(2);

	m_vFrames.emplace_back(m_Context);
	m_vFrames.emplace_back(m_Context);
}
void VulkanRenderer::CreateSwapChain()
{
	m_Context.swapchain = new VulkanSwapChain(m_Context, 1920u, 1080u, 2);
}
void VulkanRenderer::CreateCommandPool()
{
	m_CommandPool = VulkanCommandPool(m_Context);
	for (auto& frame : m_vFrames)
		frame.CommandBuffer = m_CommandPool.Allocate(CommandBufferLevel::Primary);
}

void VulkanRenderer::Submit()
{
	FrameContext& frame = m_vFrames[m_CurrentFrame];

	// -- Submit Info --
	VkSubmitInfo2 submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submitInfo.pNext = nullptr;
	submitInfo.flags = 0;

	// -- Command Buffer Submit Info --
	VkCommandBufferSubmitInfo cmdSubmitInfo{};
	cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	cmdSubmitInfo.pNext = nullptr;
	cmdSubmitInfo.commandBuffer = (*static_cast<VulkanCommandBuffer*>(frame.CommandBuffer.get()));
	cmdSubmitInfo.deviceMask = 0;
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &cmdSubmitInfo;


	// -- Wait Semaphores --
	VkSemaphoreSubmitInfo waitSemaphoreSubmitInfo{};
	waitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	waitSemaphoreSubmitInfo.pNext = nullptr;
	waitSemaphoreSubmitInfo.value = 0;
	waitSemaphoreSubmitInfo.semaphore = frame.ImageAvailableSemaphore;
	waitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitSemaphoreInfoCount = 1;
	submitInfo.pWaitSemaphoreInfos = &waitSemaphoreSubmitInfo;

	VkSemaphoreSubmitInfo signalSemaphoreSubmitInfo{};
	signalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	signalSemaphoreSubmitInfo.pNext = nullptr;
	signalSemaphoreSubmitInfo.value = 0;
	signalSemaphoreSubmitInfo.semaphore = frame.RenderFinishedSemaphore;
	submitInfo.signalSemaphoreInfoCount = 1;
	submitInfo.pSignalSemaphoreInfos = &signalSemaphoreSubmitInfo;

	// -- Submit yay --
	if (vkQueueSubmit2(m_Context.device.GetGraphicQueue(), 1, &submitInfo, frame.InFlightFences) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit Command Buffer!");
}
