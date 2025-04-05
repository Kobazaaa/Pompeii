#include "SwapChain.h"

#include <algorithm>
#include <array>
#include <stdexcept>

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  SwapChainBuilder	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Builder
//--------------------------------------------------
pom::SwapChainBuilder& pom::SwapChainBuilder::SetDesiredImageCount(uint32_t count) { m_DesiredImageCount = count; return *this; }
pom::SwapChainBuilder& pom::SwapChainBuilder::SetImageUsage(VkImageUsageFlags usage) { m_CreateInfo.imageUsage = usage; return *this; }
pom::SwapChainBuilder& pom::SwapChainBuilder::SetImageArrayLayers(uint32_t layerCount) { m_CreateInfo.imageArrayLayers = layerCount; return *this; }
void pom::SwapChainBuilder::Build(Device& device, const VmaAllocator& allocator, const PhysicalDevice& physicalDevice, const Window& window, SwapChain& swapChain, CommandPool& cmdPool)
{
	// Get Support details
	const SwapChainSupportDetails swapChainSupport = physicalDevice.GetSwapChainSupportDetails();

	// Choose
	const VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	const VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	const VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, window);

	// Set mage count for swapchain
	uint32_t imageCount = std::max(m_DesiredImageCount, swapChainSupport.capabilities.minImageCount);
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	// Setup remainders of CreateInfo
	m_CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	m_CreateInfo.surface = window.GetVulkanSurface();

	m_CreateInfo.minImageCount = imageCount;
	m_CreateInfo.imageFormat = surfaceFormat.format;
	m_CreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	m_CreateInfo.imageExtent = extent;


	pom::QueueFamilyIndices indices = physicalDevice.GetQueueFamilies();
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

	swapChain.m_OriginalBuilder = *this;
	if (vkCreateSwapchainKHR(device.GetDevice(), &m_CreateInfo, nullptr, &swapChain.m_SwapChain) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Swap Chain!");

	vkGetSwapchainImagesKHR(device.GetDevice(), swapChain.m_SwapChain, &imageCount, nullptr);
	std::vector<VkImage> vImages;
	vImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device.GetDevice(), swapChain.m_SwapChain, &imageCount, vImages.data());

	swapChain.m_vSwapChainImages.resize(imageCount);
	for (int index{}; index < imageCount; ++index)
	{
		swapChain.m_vSwapChainImages[index] = Image(vImages[index]);
	}

	swapChain.m_SwapChainImageFormat = surfaceFormat.format;
	swapChain.m_SwapChainExtent = extent;

	for (size_t index = 0; index < swapChain.GetImageCount(); ++index)
		swapChain.m_vSwapChainImages[index].GenerateImageView(device, swapChain.GetFormat(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);

	// Create Depth Resources
	VkFormat format = Image::FindSupportedFormat(physicalDevice, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	ImageBuilder imageBuilder{};
	imageBuilder.SetWidth(extent.width)
		.SetHeight(extent.height)
		.SetTiling(VK_IMAGE_TILING_OPTIMAL)
		.SetFormat(format)
		.SetUsageFlags(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		.Build(allocator, swapChain.m_DepthImage);
	swapChain.m_DepthImage.GenerateImageView(device, format, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D);
	cmdPool.TransitionImageLayout(swapChain.m_DepthImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
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
void pom::SwapChain::Destroy(Device& device, VmaAllocator& allocator)
{
	m_DepthImage.Destroy(device, allocator);

	//for (auto& framebuffer : m_vSwapChainFrameBuffers)
	//	vkDestroyFramebuffer(device.GetDevice(), framebuffer, nullptr);

	for (auto& image : m_vSwapChainImages)
		vkDestroyImageView(device.GetDevice(), image.GetImageView(), nullptr);

	vkDestroySwapchainKHR(device.GetDevice(), m_SwapChain, nullptr);
}
void pom::SwapChain::Recreate(Device& device, VmaAllocator& allocator, PhysicalDevice& physicalDevice, Window& window, CommandPool& cmdPool)
{
	auto size = window.GetSize();
	while (size.x == 0 || size.y == 0)
	{
		size = window.GetSize();
		glfwWaitEvents();
	}

	device.WaitIdle();

	Destroy(device, allocator);
	m_OriginalBuilder.Build(device, allocator, physicalDevice, window, *this, cmdPool);
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
pom::Image& pom::SwapChain::GetDepthImage()						{ return m_DepthImage; }
std::vector<pom::Image>& pom::SwapChain::GetImages()			{ return m_vSwapChainImages; }
VkSwapchainKHR& pom::SwapChain::GetSwapChain()					{ return m_SwapChain; }
uint32_t pom::SwapChain::GetImageCount()			const		{ return static_cast<uint32_t>(m_vSwapChainImages.size()); }
VkFormat pom::SwapChain::GetFormat()				const		{ return m_SwapChainImageFormat; }
VkExtent2D pom::SwapChain::GetExtent()				const		{ return m_SwapChainExtent; }
std::vector<VkImage*> pom::SwapChain::GetVulkanImages()
{
	std::vector<VkImage*> vulkanImages{};
	vulkanImages.reserve(m_vSwapChainImages.size());

	std::ranges::for_each(m_vSwapChainImages,
		[&](Image& image)
		{
			vulkanImages.emplace_back(&image.GetImage());
		});

	return vulkanImages;
}
