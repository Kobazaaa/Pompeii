// -- Pompeii Includes --
#include "VulkanCommandBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanFrameContext.h"
#include "VulkanContext.h"
#include "VulkanCommandPool.h"

// -- Standard Library --
#include <stdexcept>

using namespace pompeii;
using namespace pompeii::vulkan;

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandPool& commandPool, VulkanContext& context)
	: m_pContext(&context)
	, m_pCommandPool(&commandPool)
{

}


//--------------------------------------------------
//    Accessors
//--------------------------------------------------
ICommandPool* VulkanCommandBuffer::GetOwnerPool() const
{
	return m_pCommandPool;
}

//--------------------------------------------------
//    Functionality
//--------------------------------------------------
void VulkanCommandBuffer::Begin(bool oneTime)
{
	m_IsTemporary = oneTime;
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = oneTime ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;

	if (vkBeginCommandBuffer(m_CmdBuffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin recording Command Buffer!");
}
void VulkanCommandBuffer::End()
{
	if (vkEndCommandBuffer(m_CmdBuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to record command buffer!");
}
void VulkanCommandBuffer::Submit(FrameContext& frameCtx)
{
	VulkanFrameContext& frame = static_cast<VulkanFrameContext&>(frameCtx);

	VkSubmitInfo2 submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submitInfo.pNext = nullptr;
	submitInfo.flags = 0;

	VkCommandBufferSubmitInfo cmdSubmitInfo{};
	cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	cmdSubmitInfo.pNext = nullptr;
	cmdSubmitInfo.commandBuffer = (*static_cast<VulkanCommandBuffer*>(frame.GetCommandBuffer()));
	cmdSubmitInfo.deviceMask = 0;
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &cmdSubmitInfo;

	VkSemaphoreSubmitInfo waitSemaphoreSubmitInfo{};
	waitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	waitSemaphoreSubmitInfo.pNext = nullptr;
	waitSemaphoreSubmitInfo.value = 0;
	waitSemaphoreSubmitInfo.semaphore = frame.GetImageAvailableSemaphore();
	waitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitSemaphoreInfoCount = 1;
	submitInfo.pWaitSemaphoreInfos = &waitSemaphoreSubmitInfo;

	VkSemaphoreSubmitInfo signalSemaphoreSubmitInfo{};
	signalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	signalSemaphoreSubmitInfo.pNext = nullptr;
	signalSemaphoreSubmitInfo.value = 0;
	signalSemaphoreSubmitInfo.semaphore = frame.GetRenderFinishedSemaphore();
	submitInfo.signalSemaphoreInfoCount = 1;
	submitInfo.pSignalSemaphoreInfos = &signalSemaphoreSubmitInfo;

	if (vkQueueSubmit2(m_pContext->device.GetGraphicQueue(), 1, &submitInfo, frame.GetInFlightFence()) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit Command Buffer!");

	if (m_IsTemporary)
	{
		m_pContext->device.WaitIdle();
		m_pCommandPool->Free(this);
	}
}

void VulkanCommandBuffer::Submit()
{
	VkSubmitInfo2 submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submitInfo.pNext = nullptr;
	submitInfo.flags = 0;

	VkCommandBufferSubmitInfo cmdSubmitInfo{};
	cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	cmdSubmitInfo.pNext = nullptr;
	cmdSubmitInfo.commandBuffer = m_CmdBuffer;
	cmdSubmitInfo.deviceMask = 0;
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &cmdSubmitInfo;

	if (vkQueueSubmit2(m_pContext->device.GetGraphicQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit Command Buffer!");

	if (m_IsTemporary)
	{
		m_pContext->device.WaitIdle();
		m_pCommandPool->Free(this);
	}
}

void VulkanCommandBuffer::Clear()
{
	vkResetCommandBuffer(m_CmdBuffer, 0);
}


//--------------------------------------------------
//    Operation Overloads
//--------------------------------------------------
VulkanCommandBuffer::operator VkCommandBuffer() const
{
	return m_CmdBuffer;
}



//--------------------------------------------------
//    Commands
//--------------------------------------------------
// Data Commands
void VulkanCommandBuffer::UploadDataToBuffer(IBuffer& buffer, const void* data, size_t size, size_t offset)
{
	BufferDesc bufferDesc
	{
		.size = size,
		.usage = BufferUsage::Staging,
		.memory = MemoryType::HostVisible
	};
	VulkanBuffer stagingBuffer{ bufferDesc, *m_pContext };
	vmaCopyMemoryToAllocation(m_pContext->allocator, data, stagingBuffer, 0, size);
	CopyBuffer(stagingBuffer, buffer, 0, offset);
}
void VulkanCommandBuffer::UploadDataToImage(IImage& image, const void* data, size_t size)
{
	BufferDesc bufferDesc
	{
		.size = size,
		.usage = BufferUsage::Staging,
		.memory = MemoryType::HostVisible
	};
	VulkanBuffer stagingBuffer { bufferDesc, *m_pContext};
	vmaCopyMemoryToAllocation(m_pContext->allocator, data, stagingBuffer, 0, size);

	TransitionImageLayout(static_cast<VulkanImage&>(image),
	                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                      0, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
	                      VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	CopyBufferToImage(stagingBuffer, image);
}

// Copy Commands
void VulkanCommandBuffer::CopyBuffer(IBuffer& src, IBuffer& dst, size_t size, uint32_t srcOffset, uint32_t dstOffset )
{
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = srcOffset;
	copyRegion.dstOffset = dstOffset;
	copyRegion.size = size;

	vkCmdCopyBuffer(
		m_CmdBuffer,
		*static_cast<VulkanBuffer*>(&src),
		*static_cast<VulkanBuffer*>(&dst),
		1,
		&copyRegion
	);
}

void VulkanCommandBuffer::CopyImage(IImage& src, IImage& dst)
{
	throw std::logic_error("Not implemented");
}

void VulkanCommandBuffer::CopyBufferToImage(IBuffer& src, IImage& dst)
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
	region.imageExtent = VkExtent3D{ .width = dst.GetWidth(), .height = dst.GetHeight(), .depth = 1 };

	vkCmdCopyBufferToImage(
		m_CmdBuffer,
		*static_cast<VulkanBuffer*>(&src),
		*static_cast<VulkanImage*>(&dst),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
}

void VulkanCommandBuffer::Blit(IImage& src, IImage& dst)
{
	VkImageBlit2 blit{};
	blit.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;

	const VkExtent2D srcExtent = {.width = src.GetWidth(), .height = src.GetHeight() };
	blit.srcOffsets[0] = { .x = 0, .y = 0, .z = 0 };
	blit.srcOffsets[1] = { .x = static_cast<int32_t>(srcExtent.width), .y = static_cast<int32_t>(srcExtent.height), .z = 1 };
	blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.srcSubresource.mipLevel = 0;
	blit.srcSubresource.baseArrayLayer = 0;
	blit.srcSubresource.layerCount = 1;

	// dst
	const VkExtent2D dstExtent = { .width = dst.GetWidth(), .height = dst.GetHeight() };
	blit.dstOffsets[0] = { .x = 0, .y = 0, .z = 0 };
	blit.dstOffsets[1] = { .x = static_cast<int32_t>(dstExtent.width), .y = static_cast<int32_t>(dstExtent.height), .z = 1 };
	blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.dstSubresource.mipLevel = 0;
	blit.dstSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount = 1;

	VkBlitImageInfo2 blitInfo{};
	blitInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
	blitInfo.pNext = nullptr;

	blitInfo.srcImage = *static_cast<VulkanImage*>(&src);
	blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

	blitInfo.dstImage = *static_cast<VulkanImage*>(&dst);
	blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	blitInfo.regionCount = 1;
	blitInfo.pRegions = &blit;

	blitInfo.filter = VK_FILTER_LINEAR;

	vkCmdBlitImage2(m_CmdBuffer, &blitInfo);
}

// Image Commands
void VulkanCommandBuffer::GenerateMips(IImage& src)
{
	throw std::logic_error("Not implemented");
}

// Drawing
void VulkanCommandBuffer::DrawMesh(Mesh& pMesh)
{
	throw std::logic_error("Not implemented");
}


//--------------------------------------------------
//    Helper
//--------------------------------------------------
void VulkanCommandBuffer::TransitionImageLayout(VulkanImage& image, VkImageLayout newLayout, VkAccessFlags2 srcAccess,
	VkPipelineStageFlags2 srcStage, VkAccessFlags2 dstAccess, VkPipelineStageFlags2 dstStage) const
{
	auto format = image.GetFormat();
	bool hasDepth = format == ImageFormat::Depth24Stencil8 || format == ImageFormat::Depth32F;
	bool hasStencil = format == ImageFormat::Depth24Stencil8;

	VkImageMemoryBarrier2 barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.pNext = nullptr;

	barrier.image = image;
	barrier.oldLayout = image.m_CurrentLayout;
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

	vkCmdPipelineBarrier2(m_CmdBuffer, &dependencyInfo);

	image.m_CurrentLayout = newLayout;
}
