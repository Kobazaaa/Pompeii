#include "Image.h"
#include <stdexcept>

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Image	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::Image::Image(VkImage image) : m_Image(image){}
void pom::Image::Destroy(const Device& device, const VmaAllocator& allocator) const
{
	vkDestroyImageView(device.GetDevice(), m_ImageView, nullptr);
	vkDestroyImage(device.GetDevice(), m_Image, nullptr);
	vmaFreeMemory(allocator, m_ImageMemory);
}
VkImageView& pom::Image::GenerateImageView(const Device& device, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType viewType)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = m_Image;
	viewInfo.viewType = viewType;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = m_ImageInfo.mipLevels == 0 ? 1 : m_ImageInfo.mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = m_ImageInfo.arrayLayers == 0 ? 1 : m_ImageInfo.arrayLayers;

	if (vkCreateImageView(device.GetDevice(), &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Image View!");

	return m_ImageView;
}
VkFormat pom::Image::FindSupportedFormat(const PhysicalDevice& physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props = physicalDevice.GetFormatProperties(format);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features ||
			tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			return format;
	}

	throw std::runtime_error("Failed to find Supported Format!");
}

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkImage& pom::Image::GetImage()		  const			{ return m_Image; }
const VkImageView& pom::Image::GetImageView() const			{ return m_ImageView; }
VkFormat pom::Image::GetFormat()			  const			{ return m_ImageInfo.format; }
VkImageLayout pom::Image::GetCurrentLayout()  const			{ return m_CurrentLayout; }
bool pom::Image::HasStencilComponent()		  const			{ return m_ImageInfo.format == VK_FORMAT_D32_SFLOAT_S8_UINT || m_ImageInfo.format == VK_FORMAT_D24_UNORM_S8_UINT; }
void pom::Image::SetImageLayout(VkImageLayout newLayout)	{ m_CurrentLayout = newLayout; }


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  ImageBuilder	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::ImageBuilder::ImageBuilder()
{
	// Setup Defaults
	m_ImageInfo = {};
	m_ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;		// CAN'T CHANGE
	m_ImageInfo.imageType = VK_IMAGE_TYPE_2D;						//? CAN CHANGE
	m_ImageInfo.extent.width = 0;									//! REQUIRED CHANGE
	m_ImageInfo.extent.height = 0;									//! REQUIRED CHANGE
	m_ImageInfo.extent.depth = 1;									//? CAN CHANGE
	m_ImageInfo.mipLevels = 1;										//? CAN CHANGE
	m_ImageInfo.arrayLayers = 1;									//? CAN CHANGE
	m_ImageInfo.format = VK_FORMAT_UNDEFINED;						//! REQUIRED CHANGE
	m_ImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;					//? CAN CHANGE
	m_ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// CAN'T CHANGE
	m_ImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;		//? CAN CHANGE
	m_ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;			//? CAN CHANGE
	m_ImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;					//? CAN CHANGE
	m_ImageInfo.flags = 0;											// CAN'T CHANGE

	m_AllocInfo = {};
	m_AllocInfo.usage = VMA_MEMORY_USAGE_AUTO;						// CAN'T CHANGE
	m_AllocInfo.requiredFlags = 0;									//? CAN CHANGE

	m_UseInitialData = false;										//? CAN CHANGE
	m_pData = nullptr;												//? CAN CHANGE
	m_InitDataSize = 0;												//? CAN CHANGE
	m_InitDataHeight = 0;											//? CAN CHANGE
	m_InitDataWidth = 0;											//? CAN CHANGE
	m_InitDataOffset = 0;											//? CAN CHANGE
	m_FinalLayout = VK_IMAGE_LAYOUT_UNDEFINED;						//? CAN CHANGE
}


//--------------------------------------------------
//    Builder
//--------------------------------------------------
pom::ImageBuilder& pom::ImageBuilder::SetWidth(uint32_t width)								{ m_ImageInfo.extent.width = width; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetHeight(uint32_t height)							{ m_ImageInfo.extent.height = height; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetDepth(uint32_t depth)								{ m_ImageInfo.extent.depth = depth; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetFormat(VkFormat format)							{ m_ImageInfo.format = format; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetTiling(VkImageTiling tiling)						{ m_ImageInfo.tiling = tiling; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetUsageFlags(VkImageUsageFlags usage)				{ m_ImageInfo.usage = usage; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetMemoryProperties(VkMemoryPropertyFlags properties) { m_AllocInfo.requiredFlags = properties; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetMipLevels(uint32_t levels)							{ m_ImageInfo.mipLevels = levels; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetArrayLayers(uint32_t layers)						{ m_ImageInfo.arrayLayers = layers; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetSampleCount(VkSampleCountFlagBits sampleCount)		{ m_ImageInfo.samples = sampleCount; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetSharingMode(VkSharingMode sharingMode)				{ m_ImageInfo.sharingMode = sharingMode; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetImageType(VkImageType type)						{ m_ImageInfo.imageType = type; return *this; }
pom::ImageBuilder& pom::ImageBuilder::InitialData(void* data, uint32_t offset, uint32_t width, uint32_t height, uint32_t dataSize, VkImageLayout finalLayout)
{
	m_UseInitialData = true;
	m_pData = data;
	m_InitDataOffset = offset;
	m_InitDataSize = dataSize;
	m_InitDataHeight = height;
	m_InitDataWidth = width;
	m_ImageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	m_FinalLayout = finalLayout;
	return *this;
}

void pom::ImageBuilder::Build(const Device& device, const VmaAllocator& allocator, CommandPool& cmdPool, Image& image) const
{
	image.m_ImageInfo = m_ImageInfo;
	image.m_CurrentLayout = m_ImageInfo.initialLayout;
	if (vmaCreateImage(allocator, &m_ImageInfo, &m_AllocInfo, &image.m_Image, &image.m_ImageMemory, nullptr) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Image!");

	if (m_UseInitialData)
	{
		Buffer stagingBuffer;
		BufferAllocator stagingAllocator{};
		stagingAllocator
			.SetUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			.HostAccess(true)
			.SetSize(m_InitDataSize)
			.Allocate(device, allocator, cmdPool, stagingBuffer);
		vmaCopyMemoryToAllocation(allocator, m_pData, stagingBuffer.GetMemory(), m_InitDataOffset, m_InitDataSize);

		cmdPool.TransitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		cmdPool.CopyBufferToImage(stagingBuffer, image, m_InitDataWidth, m_InitDataHeight);
		cmdPool.TransitionImageLayout(image, m_FinalLayout);

		stagingBuffer.Destroy(device, allocator);
	}
}
