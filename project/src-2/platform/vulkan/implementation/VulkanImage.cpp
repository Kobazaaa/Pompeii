// -- Pompeii Includes --
#include "VulkanImage.h"
#include "VulkanHelpers.h"
#include "VulkanContext.h"

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
}
VulkanImage::VulkanImage(VulkanContext& context, VkImage image, VkImageView view, VkFormat format)
{
	m_pContext = &context;
	m_Image = image;
	m_Format = format;
	m_Extent = { 0, 0, 0 };
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


//--------------------------------------------------
//    Operator Overloads
//--------------------------------------------------
VulkanImage::operator VkImage()				const { return m_Image; }
