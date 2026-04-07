// -- Pompeii Includes --
#include "VulkanImage.h"
#include "VulkanHelpers.h"
#include "VulkanContext.h"
#include "VulkanCommandBuffer.h"

using namespace pompeii;
using namespace pompeii::vulkan;

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
VulkanImage::VulkanImage(const ImageDesc& imageDesc, VulkanContext& context)
	: m_pContext(&context)
	, m_ImageDesc(imageDesc)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent = {.width = imageDesc.width, .height = imageDesc.height, .depth = imageDesc.depth };
	imageInfo.mipLevels = imageDesc.mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = TO_VK_FORMAT(imageDesc.format);
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = TO_VK_USAGE(imageDesc.usage);
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.requiredFlags = TO_VK_MEMORY(imageDesc.memory);

	vmaCreateImage(
		context.allocator,
		&imageInfo,
		&allocInfo,
		&m_Image,
		&m_Memory,
		nullptr
	);

	CreateImageView();
}
VulkanImage::VulkanImage(VulkanContext& context, VkImage image, VkImageView view, VkFormat format, VkExtent2D extent)
{
	m_pContext = &context;
	m_Image = image;
	m_View = view;

	m_ImageDesc = {};
	m_ImageDesc.isDepthImage = false;

	m_ImageDesc.width = extent.width;
	m_ImageDesc.height = extent.height;
	m_ImageDesc.depth = 1;

	m_ImageDesc.memory = MemoryType::DeviceLocal;
	m_ImageDesc.usage = ImageUsage::ColorAttachment;
	m_ImageDesc.mipLevels = 1;

	m_ImageDesc.format = TO_POMPEII_FORMAT(format);
}

VulkanImage::~VulkanImage()
{
	if (m_Image && m_Memory)
		vmaDestroyImage(m_pContext->allocator, m_Image, m_Memory);
}

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
uint32_t VulkanImage::GetWidth()			const { return m_ImageDesc.width; }
uint32_t VulkanImage::GetHeight()			const { return m_ImageDesc.height; }
ImageFormat VulkanImage::GetFormat()		const { return m_ImageDesc.format; }
ImageUsage VulkanImage::GetUsage()			const { return m_ImageDesc.usage; }
MemoryType VulkanImage::GetMemoryType()		const { return m_ImageDesc.memory; }

VkImageLayout VulkanImage::GetLayout()		const { return m_CurrentLayout; }


//--------------------------------------------------
//    Operator Overloads
//--------------------------------------------------
VulkanImage::operator VkImage()				const { return m_Image; }
VulkanImage::operator VkImageView()			const { return m_View; }
VulkanImage::operator VmaAllocation()		const { return m_Memory; }


//--------------------------------------------------
//    Helper
//--------------------------------------------------
void VulkanImage::CreateImageView()
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = m_Image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = TO_VK_FORMAT(m_ImageDesc.format);
	viewInfo.subresourceRange.aspectMask = m_ImageDesc.isDepthImage ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(m_pContext->device, &viewInfo, nullptr, &m_View) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Image View!");
}

void VulkanImage::TransitionImageLayout(const VulkanCommandBuffer& cmd, VkImageLayout newLayout,
	VkAccessFlags2 srcAccess, VkPipelineStageFlags2 srcStage,
	VkAccessFlags2 dstAccess, VkPipelineStageFlags2 dstStage) const
{
	auto format = GetFormat();
	bool hasDepth = format == ImageFormat::Depth24Stencil8 || format == ImageFormat::Depth32F;
	bool hasStencil = format == ImageFormat::Depth24Stencil8;

	VkImageMemoryBarrier2 barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.pNext = nullptr;

	barrier.image = m_Image;
	barrier.oldLayout = m_CurrentLayout;
	barrier.newLayout = newLayout;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.srcAccessMask = srcAccess;
	barrier.srcStageMask = srcStage;

	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstAccessMask = dstAccess;
	barrier.dstStageMask = dstStage;

	barrier.subresourceRange.aspectMask = 0;
	if (hasDepth)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (hasStencil) barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkDependencyInfo dependencyInfo{};
	dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependencyInfo.dependencyFlags = 0;
	dependencyInfo.pNext = nullptr;
	dependencyInfo.memoryBarrierCount = 0;
	dependencyInfo.pMemoryBarriers = nullptr;
	dependencyInfo.bufferMemoryBarrierCount = 0;
	dependencyInfo.pBufferMemoryBarriers = nullptr;
	dependencyInfo.imageMemoryBarrierCount = 1;
	dependencyInfo.pImageMemoryBarriers = &barrier;

	vkCmdPipelineBarrier2(cmd, &dependencyInfo);

	m_CurrentLayout = newLayout;
}
