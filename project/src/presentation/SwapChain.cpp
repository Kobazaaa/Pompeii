// -- Standard Library --
#include <algorithm>
#include <stdexcept>

// -- Pompeii Includes --
#include "SwapChain.h"
#include "Context.h"
#include "Window.h"
#include "CommandPool.h"
#include "Debugger.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  SwapChainBuilder	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Builder
//--------------------------------------------------
pom::SwapChainBuilder& pom::SwapChainBuilder::SetDesiredImageCount(uint32_t count) { m_DesiredImageCount = count; return *this; }
pom::SwapChainBuilder& pom::SwapChainBuilder::SetImageUsage(VkImageUsageFlags usage) { m_CreateInfo.imageUsage = usage; return *this; }
pom::SwapChainBuilder& pom::SwapChainBuilder::SetImageArrayLayers(uint32_t layerCount) { m_CreateInfo.imageArrayLayers = layerCount; return *this; }
void pom::SwapChainBuilder::Build(Context& context, const Window& window, SwapChain& swapChain)
{
	// -- Get Support Details --
	const SwapChainSupportDetails swapChainSupport = context.physicalDevice.GetSwapChainSupportDetails(window.GetVulkanSurface());

	// -- Choose --
	const VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	const VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	const VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, window);

	// -- Set Image Count --
	uint32_t imageCount = std::max(m_DesiredImageCount, swapChainSupport.capabilities.minImageCount);
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	// -- Setup remainders of CreateInfo --
	m_CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	m_CreateInfo.surface = window.GetVulkanSurface();

	m_CreateInfo.minImageCount = imageCount;
	m_CreateInfo.imageFormat = surfaceFormat.format;
	m_CreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	m_CreateInfo.imageExtent = extent;


	pom::QueueFamilyIndices indices = context.physicalDevice.GetQueueFamilies();
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	if (indices.graphicsFamily != indices.presentFamily)
	{
		m_CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		m_CreateInfo.queueFamilyIndexCount = 2;
		m_CreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		m_CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		m_CreateInfo.queueFamilyIndexCount = 0;
		m_CreateInfo.pQueueFamilyIndices = nullptr;
	}

	m_CreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	m_CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	m_CreateInfo.presentMode = presentMode;
	m_CreateInfo.clipped = VK_TRUE;
	m_CreateInfo.oldSwapchain = VK_NULL_HANDLE;

	// -- Create the SwapChainKHR --
	swapChain.m_OriginalBuilder = *this;
	if (vkCreateSwapchainKHR(context.device.GetHandle(), &m_CreateInfo, nullptr, &swapChain.m_SwapChain) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Swap Chain!");

	// -- Retrieve the SwapChain Images --
	vkGetSwapchainImagesKHR(context.device.GetHandle(), swapChain.m_SwapChain, &imageCount, nullptr);
	std::vector<VkImage> swapChainImages;
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(context.device.GetHandle(), swapChain.m_SwapChain, &imageCount, swapChainImages.data());

	swapChain.m_SwapChainImageFormat = surfaceFormat.format;
	swapChain.m_SwapChainExtent = extent;
	swapChain.m_vSwapChainImages.resize(imageCount);

	// -- Create the SwapChain Image & Views --
	swapChain.m_vSwapChainImages.resize(imageCount);
	for (size_t index = 0; index < swapChainImages.size(); ++index)
	{
		std::string name = "Swapchain_Image_" + std::to_string(index);
		ImageBuilder builder{};
		builder
			.SetDebugName(name.c_str())
			.SetWidth(extent.width)
			.SetHeight(extent.height)
			.SetFormat(swapChain.m_SwapChainImageFormat)
			.SetPreMadeImage(swapChainImages[index])
			.Build(context, swapChain.m_vSwapChainImages[index]);
		swapChain.m_vSwapChainImages[index].CreateView(context, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);
	}
}

//--------------------------------------------------
//    Helpers
//--------------------------------------------------
VkExtent2D pom::SwapChainBuilder::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window& window)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else
	{
		const auto size = window.GetSize();

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(size.x),
			static_cast<uint32_t>(size.y)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}
VkSurfaceFormatKHR pom::SwapChainBuilder::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}

	return availableFormats[0];
}
VkPresentModeKHR pom::SwapChainBuilder::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	// See if VK_PRESENT_MODE_MAILBOX_KHR is available
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
	}

	// Only VK_PRESENT_MODE_FIFO_KHR is GUARANTEED to be present! 
	return VK_PRESENT_MODE_FIFO_KHR;
}


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  SwapChain
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::SwapChain::Destroy(const Context& context)
{
	for (Image& image : m_vSwapChainImages)
		image.DestroyAllViews(context);

	vkDestroySwapchainKHR(context.device.GetHandle(), m_SwapChain, nullptr);
}
void pom::SwapChain::Recreate(Context& context, const Window& window)
{
	m_OriginalBuilder.Build(context, window, *this);
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkSwapchainKHR& pom::SwapChain::GetHandle()	const		{ return m_SwapChain; }
std::vector<pom::Image>& pom::SwapChain::GetImages()			{ return m_vSwapChainImages; }

uint32_t pom::SwapChain::GetImageCount()			const		{ return static_cast<uint32_t>(m_vSwapChainImages.size()); }

VkFormat pom::SwapChain::GetFormat()				const		{ return m_SwapChainImageFormat; }
VkExtent2D pom::SwapChain::GetExtent()				const		{ return m_SwapChainExtent; }
