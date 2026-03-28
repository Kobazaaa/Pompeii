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
}

//--------------------------------------------------
//    Functionality
//--------------------------------------------------
void VulkanImage::SetData(const void* data, size_t size, uint32_t mipLevel, uint32_t arrayLayer)
{
	VkBuffer stagingBuffer;
	VmaAllocation stagingAlloc;

	void* mapped;
	vmaMapMemory(m_pContext->allocator, stagingAlloc, &mapped);
	memcpy(mapped, data, size);
	vmaUnmapMemory(m_pContext->allocator, stagingAlloc);
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
uint32_t VulkanImage::GetWidth()			const { return m_ImageDesc.width; }
uint32_t VulkanImage::GetHeight()			const { return m_ImageDesc.height; }
ImageFormat VulkanImage::GetFormat()		const { return m_ImageDesc.format; }
ImageUsage VulkanImage::GetUsage()			const { return m_ImageDesc.usage; }
MemoryType VulkanImage::GetMemoryType()		const { return m_ImageDesc.memory; }

VkImage VulkanImage::GetHandle()			const { return m_Image; }
