// -- Pompeii Includes --
#include "SwapChain.h"
#include "VulkanContext.h"
#include "VulkanFrameContext.h"

// -- Standard Library --
#include <algorithm>
#include <stdexcept>

using namespace pompeii;
using namespace pompeii::vulkan;

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
VulkanSwapChain::VulkanSwapChain(VulkanContext& context, uint32_t width, uint32_t height, uint32_t buffers)
	: m_pContext(&context)
{
	CreateSwapChain(width, height, buffers);
	CreateImageViews();
	CreateBackBuffers();
}
VulkanSwapChain::~VulkanSwapChain()
{
	CleanUp();
}


//--------------------------------------------------
//    Functionality
//--------------------------------------------------
bool VulkanSwapChain::AcquireNextImage(const VulkanFrameContext& frame)
{
	vkWaitForFences(m_pContext->device, 1, &frame.InFlightFences, VK_TRUE, UINT64_MAX);

	VkResult result = vkAcquireNextImageKHR(
		m_pContext->device,
		m_SwapChain,
		UINT64_MAX,
		frame.ImageAvailableSemaphore,
		VK_NULL_HANDLE,
		&m_CurrentImageIndex
	);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		Resize(m_Extent.width, m_Extent.height);
		return false;
	}
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to acquire Swap Chain Image");

	// -- Reset Fence to be un-signaled (not done) --
	vkResetFences(m_pContext->device, 1, &frame.InFlightFences);

	return true;
}
IImage* VulkanSwapChain::GetCurrentBackBuffer()
{
	return &m_vBackBuffers[m_CurrentImageIndex];
}
bool VulkanSwapChain::Present(const VulkanFrameContext& frame)
{
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &frame.RenderFinishedSemaphore;

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_SwapChain;
	presentInfo.pImageIndices = &m_CurrentImageIndex;
	presentInfo.pResults = nullptr;

	auto result = vkQueuePresentKHR(m_pContext->device.GetPresentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		Resize(m_Extent.width, m_Extent.height);
		return false;
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to present Swap Chain Image!");

	return true;
}
void VulkanSwapChain::Resize(uint32_t width, uint32_t height)
{
	vkDeviceWaitIdle(m_pContext->device);

	CleanUp();

	CreateSwapChain(width, height, 2); //todo not hardcoded 2 lol
	CreateImageViews();
	CreateBackBuffers();
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
uint32_t VulkanSwapChain::GetImageCount() const
{
	throw std::logic_error("Not implemented");
}
uint32_t VulkanSwapChain::GetCurrentImageIndex() const
{
	throw std::logic_error("Not implemented");
}


//--------------------------------------------------
//    Helpers - Creation
//--------------------------------------------------
void VulkanSwapChain::CreateSwapChain(uint32_t width, uint32_t height, uint32_t buffers)
{
	SwapChainDetails swapChainDetails{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_pContext->physicalDevice, m_pContext->surface, &swapChainDetails.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_pContext->physicalDevice, m_pContext->surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		swapChainDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_pContext->physicalDevice, m_pContext->surface, &formatCount, swapChainDetails.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_pContext->physicalDevice, m_pContext->surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		swapChainDetails.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_pContext->physicalDevice, m_pContext->surface, &presentModeCount, swapChainDetails.presentModes.data());
	}

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainDetails.formats);
	const VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainDetails.presentModes);
	m_Extent = ChooseSwapExtent(swapChainDetails.capabilities, VkExtent2D{width, height});

	uint32_t imageCount = std::max(buffers, swapChainDetails.capabilities.minImageCount);
	if (swapChainDetails.capabilities.maxImageCount > 0 && imageCount > swapChainDetails.capabilities.maxImageCount)
		imageCount = swapChainDetails.capabilities.maxImageCount;


	VkSwapchainCreateInfoKHR createInfo{ };
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_pContext->surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = m_Extent;

	QueueFamilyIndices indices = m_pContext->physicalDevice.GetQueueFamilies();
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainDetails.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkResult result = vkCreateSwapchainKHR(m_pContext->device, &createInfo, nullptr, &m_SwapChain);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create Swap Chain!");

	vkGetSwapchainImagesKHR(m_pContext->device, m_SwapChain, &imageCount, nullptr);
	m_vImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_pContext->device, m_SwapChain, &imageCount, m_vImages.data());

	m_Format = createInfo.imageFormat;
}
void VulkanSwapChain::CreateImageViews()
{
	m_vImageViews.resize(m_vImages.size());

	for (size_t i = 0; i < m_vImages.size(); i++)
	{
		VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = m_vImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_Format;

		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_pContext->device, &viewInfo, nullptr, &m_vImageViews[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image view");
	}
}
void VulkanSwapChain::CreateBackBuffers()
{
	m_vBackBuffers.reserve(m_vImages.size());
	for (size_t i = 0; i < m_vImages.size(); i++)
	{
		m_vBackBuffers.emplace_back(
			*m_pContext,
			m_vImages[i],
			m_vImageViews[i],
			m_Format
		);
	}
}
void VulkanSwapChain::CleanUp()
{
	for (auto view : m_vImageViews)
		vkDestroyImageView(m_pContext->device, view, nullptr);
	vkDestroySwapchainKHR(m_pContext->device, m_SwapChain, nullptr);
	m_vBackBuffers.clear();
}

//--------------------------------------------------
//    Helpers - Choosing
//--------------------------------------------------
VkExtent2D VulkanSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,	VkExtent2D windowExtent)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else
	{
		VkExtent2D actualExtent = windowExtent;

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}
VkSurfaceFormatKHR VulkanSwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}

	return availableFormats[0];
}
VkPresentModeKHR VulkanSwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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
