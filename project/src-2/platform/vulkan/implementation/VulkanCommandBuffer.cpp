// -- Pompeii Includes --
#include "VulkanCommandBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"

// -- Standard Library --
#include <stdexcept>

using namespace pompeii;
using namespace pompeii::vulkan;

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
VulkanCommandBuffer::VulkanCommandBuffer(ICommandPool& commandPool)
	: m_pCommandPool(&commandPool)
{

}


//--------------------------------------------------
//    Accessors
//--------------------------------------------------
pompeii::ICommandPool* VulkanCommandBuffer::GetOwnerPool() const
{
	return m_pCommandPool;
}

//--------------------------------------------------
//    Functionality
//--------------------------------------------------
void VulkanCommandBuffer::Begin()
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;

	if (vkBeginCommandBuffer(m_CmdBuffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin recording Command Buffer!");
}
void VulkanCommandBuffer::End()
{
	if (vkEndCommandBuffer(m_CmdBuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to record command buffer!");
}

void VulkanCommandBuffer::Clear()
{
	vkResetCommandBuffer(m_CmdBuffer, 0);
}


//--------------------------------------------------
//    Commands
//--------------------------------------------------
// Copy Commands
void VulkanCommandBuffer::CopyBuffer(IBuffer* src, IBuffer* dst, size_t size, uint32_t srcOffset, uint32_t dstOffset )
{
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = srcOffset;
	copyRegion.dstOffset = dstOffset;
	copyRegion.size = size;

	vkCmdCopyBuffer(
		m_CmdBuffer,
		static_cast<VulkanBuffer*>(src)->GetHandle(),
		static_cast<VulkanBuffer*>(dst)->GetHandle(),
		1,
		&copyRegion
	);
}

void VulkanCommandBuffer::CopyImage(IImage* src, IImage* dst)
{
	throw std::logic_error("Not implemented");
}

void VulkanCommandBuffer::CopyBufferToImage(IBuffer* src, IImage* dst)
{
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { .x = 0, .y = 0, .z = 0 };
	region.imageExtent = VkExtent3D{ .width = dst->GetWidth(), .height = dst->GetHeight(), .depth = 1 };

	vkCmdCopyBufferToImage(
		m_CmdBuffer,
		static_cast<VulkanBuffer*>(src)->GetHandle(),
		static_cast<VulkanImage*>(dst)->GetHandle(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
}

void VulkanCommandBuffer::Blit(IImage* src, IImage* dst)
{
	VkImageBlit2 blit{};
	blit.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;

	const VkExtent2D srcExtent = {.width = src->GetWidth(), .height = src->GetHeight() };
	blit.srcOffsets[0] = { .x = 0, .y = 0, .z = 0 };
	blit.srcOffsets[1] = { .x = static_cast<int32_t>(srcExtent.width), .y = static_cast<int32_t>(srcExtent.height), .z = 1 };
	blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.srcSubresource.mipLevel = 0;
	blit.srcSubresource.baseArrayLayer = 0;
	blit.srcSubresource.layerCount = 1;

	// dst
	const VkExtent2D dstExtent = { .width = dst->GetWidth(), .height = dst->GetHeight() };
	blit.dstOffsets[0] = { .x = 0, .y = 0, .z = 0 };
	blit.dstOffsets[1] = { .x = static_cast<int32_t>(dstExtent.width), .y = static_cast<int32_t>(dstExtent.height), .z = 1 };
	blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.dstSubresource.mipLevel = 0;
	blit.dstSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount = 1;

	VkBlitImageInfo2 blitInfo{};
	blitInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
	blitInfo.pNext = nullptr;

	blitInfo.srcImage = static_cast<VulkanImage*>(src)->GetHandle();
	blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

	blitInfo.dstImage = static_cast<VulkanImage*>(dst)->GetHandle();
	blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	blitInfo.regionCount = 1;
	blitInfo.pRegions = &blit;

	blitInfo.filter = VK_FILTER_LINEAR;

	vkCmdBlitImage2(m_CmdBuffer, &blitInfo);
}

// Image Commands
void VulkanCommandBuffer::GenerateMips(IImage* src)
{
	throw std::logic_error("Not implemented");
}

// Drawing
void VulkanCommandBuffer::DrawMesh(Mesh* pMesh)
{
	throw std::logic_error("Not implemented");
}
