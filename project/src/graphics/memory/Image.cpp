// -- Standard Library --
#include <stdexcept>
#include <algorithm>

// -- Pompeii Includes --
#include "Image.h"
#include "Context.h"
#include "CommandPool.h"
#include "Buffer.h"
#include "Debugger.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Image View	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::ImageView::Destroy(const Context& context) const
{
	if (m_ImageView)
		vkDestroyImageView(context.device.GetHandle(), m_ImageView, nullptr);
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkImageView& pom::ImageView::GetHandle()	const { return m_ImageView; }
const pom::Image& pom::ImageView::GetImage()	const { return *m_pOwnerImage; }

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Image	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::Image::Image(Image&& other) noexcept
{
	m_Image = std::move(other.m_Image);
	other.m_Image = VK_NULL_HANDLE;
	m_vImageViews = std::move(other.m_vImageViews);
	other.m_vImageViews.clear();
	m_ImageMemory = std::move(other.m_ImageMemory);
	other.m_ImageMemory = VK_NULL_HANDLE;
	m_CurrentLayout = std::move(other.m_CurrentLayout);
	other.m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	m_ImageInfo = std::move(other.m_ImageInfo);
	other.m_ImageInfo = {};
}
pom::Image& pom::Image::operator=(Image&& other) noexcept
{
	if (this == &other)
		return *this;
	m_Image = std::move(other.m_Image);
	other.m_Image = VK_NULL_HANDLE;
	m_vImageViews = std::move(other.m_vImageViews);
	other.m_vImageViews.clear();
	m_ImageMemory = std::move(other.m_ImageMemory);
	other.m_ImageMemory = VK_NULL_HANDLE;
	m_CurrentLayout = std::move(other.m_CurrentLayout);
	other.m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	m_ImageInfo = std::move(other.m_ImageInfo);
	other.m_ImageInfo = {};
	return *this;
}

void pom::Image::Destroy(const Context& context)
{
	DestroyAllViews(context);
	if (m_ImageMemory)
		vmaDestroyImage(context.allocator, m_Image, m_ImageMemory);
}
void pom::Image::DestroyAllViews(const Context& context)
{
	for (auto& view : m_vImageViews)
		view.Destroy(context);
	m_vImageViews.clear();
}
void pom::Image::DestroyViewsFrom(const Context& context, uint32_t firstViewToRemove)
{
	if (firstViewToRemove > static_cast<uint32_t>(m_vImageViews.size() - 1))
		return;

	for (uint32_t i{ firstViewToRemove }; i < m_vImageViews.size(); ++i)
		m_vImageViews[i].Destroy(context);
	m_vImageViews.erase(m_vImageViews.begin() + firstViewToRemove, m_vImageViews.end());
}

pom::ImageView& pom::Image::CreateView(const Context& context, VkImageAspectFlags aspectFlags, VkImageViewType viewType,
                                       uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount)
{
	m_vImageViews.emplace_back();
	m_vImageViews.back().m_pOwnerImage = this;

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = m_Image;
	viewInfo.viewType = viewType;
	viewInfo.format = m_ImageInfo.format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = baseMip;
	viewInfo.subresourceRange.levelCount = mipCount;
	viewInfo.subresourceRange.baseArrayLayer = baseLayer;
	viewInfo.subresourceRange.layerCount = layerCount;
	m_vImageViews.back().m_Info = viewInfo;

	if (vkCreateImageView(context.device.GetHandle(), &viewInfo, nullptr, &m_vImageViews.back().m_ImageView) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Image View!");

	return m_vImageViews.back();
}
VkFormat pom::Image::FindSupportedFormat(const PhysicalDevice& physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (const VkFormat format : candidates)
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
const VkImage& pom::Image::GetHandle()							const { return m_Image; }
const pom::ImageView& pom::Image::GetView(uint32_t idx)			const { return m_vImageViews.at(idx); }
uint32_t pom::Image::GetViewCount()								const { return static_cast<uint32_t>(m_vImageViews.size()); }
const std::vector<pom::ImageView>& pom::Image::GetAllViews()	const { return m_vImageViews; }

uint32_t pom::Image::GetMipLevels()				const		{ return m_ImageInfo.mipLevels; }
uint32_t pom::Image::GetLayerCount()			const		{ return m_ImageInfo.arrayLayers; }

VkFormat pom::Image::GetFormat()				const		{ return m_ImageInfo.format; }
VkExtent3D pom::Image::GetExtent3D()			const		{ return m_ImageInfo.extent; }
VkExtent2D pom::Image::GetExtent2D()			const		{ return VkExtent2D{ m_ImageInfo.extent.width, m_ImageInfo.extent.height}; }
VkImageLayout pom::Image::GetCurrentLayout()	const		{ return m_CurrentLayout; }
bool pom::Image::HasStencilComponent()			const
{
	switch (m_ImageInfo.format)
	{
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
		return true;
	default:
		return false;
	}
}
bool pom::Image::HasDepthComponent()			const
{
	switch (m_ImageInfo.format)
	{
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_X8_D24_UNORM_PACK32:
	case VK_FORMAT_D32_SFLOAT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		return true;
	default:
		return false;
	}
}


//--------------------------------------------------
//    Commands
//--------------------------------------------------
void pom::Image::TransitionLayout(const CommandBuffer& cmd, VkImageLayout newLayout,
								  VkAccessFlags2 srcAccess, VkPipelineStageFlags2 srcStage,
								  VkAccessFlags2 dstAccess, VkPipelineStageFlags2 dstStage,
								  uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount)
{
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
	if (HasDepthComponent())
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (HasStencilComponent()) barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = baseMip;
	barrier.subresourceRange.levelCount = mipCount;
	barrier.subresourceRange.baseArrayLayer = baseLayer;
	barrier.subresourceRange.layerCount = layerCount;

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

	vkCmdPipelineBarrier2(cmd.GetHandle(), &dependencyInfo);

	m_CurrentLayout = newLayout;
}

void pom::Image::InsertBarrier(const CommandBuffer& cmd,
							   VkAccessFlags2 srcAccess, VkPipelineStageFlags2 srcStage,
							   VkAccessFlags2 dstAccess, VkPipelineStageFlags2 dstStage)
{
	TransitionLayout(cmd, m_CurrentLayout, srcAccess, srcStage, dstAccess, dstStage, 0, GetMipLevels(), 0, GetLayerCount());
}

void pom::Image::GenerateMipMaps(const Context& context, const CommandBuffer& cmd, uint32_t texW, uint32_t texH, uint32_t mips, VkImageLayout finalLayout)
{
	// -- Support --
	VkFormat imageFormat = GetFormat();
	VkFormatProperties formatProperties = context.physicalDevice.GetFormatProperties(imageFormat);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		throw std::runtime_error("texture image format does not support linear blitting!");
	if (!(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		throw std::runtime_error("texture image format does not support linear blitting!");

	// -- Setup --
	VkAccessFlags2 finalDstAccessFlags{};
	VkPipelineStageFlags2 finalDstStageFlags{};
	if (finalLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		finalDstAccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
		finalDstStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (finalLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		finalDstAccessFlags = VK_ACCESS_SHADER_READ_BIT;
		finalDstStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		finalDstAccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		finalDstStageFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}

	// -- Blit --
	VkImageMemoryBarrier2 barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.pNext = nullptr;

	barrier.image = m_Image;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;				// Gets Overwritten
	barrier.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;				// Gets Overwritten

	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;	// Gets Overwritten
	barrier.dstAccessMask = 0;									// Gets Overwritten

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;	// Gets Overwritten
	barrier.srcAccessMask = 0;									// Gets Overwritten

	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;					// Gets Overwritten
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	int32_t mipWidth = static_cast<int32_t>(texW);
	int32_t mipHeight = static_cast<int32_t>(texH);
	for (uint32_t mip{ 1 }; mip < mips; ++mip)
	{
		// -- Setup Barrier --
		barrier.subresourceRange.baseMipLevel = mip - 1;

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;


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

		vkCmdPipelineBarrier2(cmd.GetHandle(), &dependencyInfo);

		// -- Blit --
		VkImageBlit2 blit{};
		blit.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;

		// src
		blit.srcOffsets[0] = { .x = 0, .y = 0, .z = 0 };
		blit.srcOffsets[1] = { .x = mipWidth, .y = mipHeight, .z = 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = mip - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;

		// dst
		blit.dstOffsets[0] = { .x = 0, .y = 0, .z = 0 };
		blit.dstOffsets[1] = { .x = mipWidth > 1 ? mipWidth / 2 : 1, .y = mipHeight > 1 ? mipHeight / 2 : 1, .z = 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = mip;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		VkBlitImageInfo2 blitInfo{};
		blitInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
		blitInfo.pNext = nullptr;

		blitInfo.srcImage = m_Image;
		blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		blitInfo.dstImage = m_Image;
		blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		blitInfo.regionCount = 1;
		blitInfo.pRegions = &blit;

		blitInfo.filter = VK_FILTER_LINEAR;

		vkCmdBlitImage2(cmd.GetHandle(), &blitInfo);

		// -- Barrier --
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

		barrier.newLayout = finalLayout;
		barrier.dstAccessMask = finalDstAccessFlags;
		barrier.dstStageMask = finalDstStageFlags;


		dependencyInfo = {};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.dependencyFlags = 0;
		dependencyInfo.pNext = nullptr;
		dependencyInfo.memoryBarrierCount = 0;
		dependencyInfo.pMemoryBarriers = nullptr;
		dependencyInfo.bufferMemoryBarrierCount = 0;
		dependencyInfo.pBufferMemoryBarriers = nullptr;
		dependencyInfo.imageMemoryBarrierCount = 1;
		dependencyInfo.pImageMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2(cmd.GetHandle(), &dependencyInfo);

		// -- Next Mip --
		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	// -- Barrier --
	barrier.subresourceRange.baseMipLevel = mips - 1;

	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

	barrier.newLayout = finalLayout;
	barrier.dstAccessMask = finalDstAccessFlags;
	barrier.dstStageMask = finalDstStageFlags;


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

	vkCmdPipelineBarrier2(cmd.GetHandle(), &dependencyInfo);

	m_CurrentLayout = finalLayout;
}


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
	m_ImageInfo.flags = 0;											//? CAN CHANGE

	m_AllocInfo = {};
	m_AllocInfo.usage = VMA_MEMORY_USAGE_AUTO;						// CAN'T CHANGE
	m_AllocInfo.requiredFlags = 0;									//? CAN CHANGE

	m_PreMadeImage = VK_NULL_HANDLE;								//? CAN CHANGE
	m_pName = nullptr;												//? CAN CHANGE
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
pom::ImageBuilder& pom::ImageBuilder::SetDebugName(const char* name)						{ m_pName = name; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetWidth(uint32_t width)								{ m_ImageInfo.extent.width = width; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetHeight(uint32_t height)							{ m_ImageInfo.extent.height = height; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetDepth(uint32_t depth)								{ m_ImageInfo.extent.depth = depth; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetFormat(VkFormat format)							{ m_ImageInfo.format = format; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetTiling(VkImageTiling tiling)						{ m_ImageInfo.tiling = tiling; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetUsageFlags(VkImageUsageFlags usage)				{ m_ImageInfo.usage = usage; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetCreateFlags(VkImageCreateFlags flags)				{ m_ImageInfo.flags |= flags; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetMemoryProperties(VkMemoryPropertyFlags properties) { m_AllocInfo.requiredFlags = properties; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetMipLevels(uint32_t levels)							{ m_ImageInfo.mipLevels = levels; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetArrayLayers(uint32_t layers)						{ m_ImageInfo.arrayLayers = layers; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetSampleCount(VkSampleCountFlagBits sampleCount)		{ m_ImageInfo.samples = sampleCount; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetSharingMode(VkSharingMode sharingMode)				{ m_ImageInfo.sharingMode = sharingMode; return *this; }
pom::ImageBuilder& pom::ImageBuilder::SetImageType(VkImageType type)						{ m_ImageInfo.imageType = type; return *this; }
pom::ImageBuilder& pom::ImageBuilder::InitialData(void* data, uint32_t offset, uint32_t
													width, uint32_t height, uint32_t dataSize,
													VkImageLayout finalLayout)
{
	m_UseInitialData = true;
	m_pData = data;
	m_InitDataOffset = offset;
	m_InitDataSize = dataSize;
	m_InitDataHeight = height;
	m_InitDataWidth = width;
	m_ImageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	if (m_ImageInfo.mipLevels > 1)
		m_ImageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	m_FinalLayout = finalLayout;
	return *this;
}
pom::ImageBuilder& pom::ImageBuilder::SetPreMadeImage(VkImage image)
{
	m_PreMadeImage = image;
	return *this;
}

void pom::ImageBuilder::Build(const Context& context, Image& image) const
{
	image.m_ImageInfo = m_ImageInfo;
	image.m_CurrentLayout = m_ImageInfo.initialLayout;
	image.m_Image = m_PreMadeImage;
	if (image.m_Image == VK_NULL_HANDLE)
	{
		if (vmaCreateImage(context.allocator, &m_ImageInfo, &m_AllocInfo, &image.m_Image, &image.m_ImageMemory, nullptr) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Image!");
	}

	if (m_UseInitialData)
	{
		Buffer stagingBuffer;
		BufferAllocator stagingAllocator{};
		stagingAllocator
			.SetUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			.HostAccess(true)
			.SetSize(m_InitDataSize)
			.Allocate(context, stagingBuffer);
		vmaCopyMemoryToAllocation(context.allocator, m_pData, stagingBuffer.GetMemoryHandle(), m_InitDataOffset, m_InitDataSize);

		CommandBuffer& cmd = context.commandPool->AllocateCmdBuffers(1);
		cmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		{
			image.TransitionLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
								   0, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
								   VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
								   0, m_ImageInfo.mipLevels, 0, 1);
			stagingBuffer.CopyToImage(cmd, image, VkExtent3D{ m_InitDataWidth, m_InitDataHeight, 1 }, 0, 0, 1);
			image.GenerateMipMaps(context, cmd, m_InitDataWidth, m_InitDataHeight, m_ImageInfo.mipLevels, m_FinalLayout);
		}
		cmd.End();
		cmd.Submit(context.device.GetGraphicQueue(), true);
		cmd.Free(context.device);
		stagingBuffer.Destroy(context);
	}

	if (m_pName)
	{
		Debugger::SetDebugObjectName(reinterpret_cast<uint64_t>(image.GetHandle()), VK_OBJECT_TYPE_IMAGE, m_pName);
	}
}
