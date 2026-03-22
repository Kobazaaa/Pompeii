// -- Pompeii Includes --
#include "VulkanRenderer.h"

// -- Standard Library --
#include <stdexcept>

using namespace pompeii;
using namespace pompeii::vulkan;

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
VulkanRenderer::VulkanRenderer()
{
	InitializeVulkan();
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
	if (!m_Context.swapchain.AcquireNextImage())
		return;
}

void VulkanRenderer::Execute()
{
	// Record frame
	const auto& cmd = m_vCommandBuffers[m_CurrentFrame];
	cmd->Begin();
	for (const auto& pass : m_vRenderPasses)
	{
		pass->Execute(cmd.get());
	}
}

void VulkanRenderer::EndFrame()
{
	const auto& cmd = m_vCommandBuffers[m_CurrentFrame];
	cmd->End();
	// submit command buffer

	m_Context.swapchain.Present();
	m_CurrentFrame = (m_CurrentFrame + 1) % m_vCommandBuffers.size();
}


//--------------------------------------------------
//    Helpers
//--------------------------------------------------
void VulkanRenderer::InitializeVulkan()
{
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
	ChoosePhysicalDevice(features2);
	CreateDevice(features2);
	CreateAllocator();
	CreateCommandPool();
	CreateSwapChain();
}

void VulkanRenderer::CreateInstance()
{
	vulkan::InstanceBuilder builder{};
	builder
		.SetApplicationName("Pompeii")
		.SetEngineName("PompeiiEngine")
		.SetAPIVersion(VK_API_VERSION_1_3)
		.Build(m_Context);
}
void VulkanRenderer::CreateDevice(VkPhysicalDeviceFeatures2 features)
{
	vulkan::DeviceBuilder builder{};
	builder
		.SetFeatures(features)
		.Build(m_Context);
}
void VulkanRenderer::ChoosePhysicalDevice(VkPhysicalDeviceFeatures2 features)
{
	vulkan::PhysicalDeviceSelector selector;
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
void VulkanRenderer::CreateSwapChain()
{
	m_Context.swapchain = vulkan::VulkanSwapChain(m_Context, 1920u, 1080u, 2);
}
void VulkanRenderer::CreateCommandPool()
{
	m_CommandPool = VulkanCommandPool(m_Context);

	m_vCommandBuffers.push_back(m_CommandPool.Allocate(CommandBufferLevel::Primary));
	m_vCommandBuffers.push_back(m_CommandPool.Allocate(CommandBufferLevel::Primary));
	m_vCommandBuffers.push_back(m_CommandPool.Allocate(CommandBufferLevel::Primary));
}