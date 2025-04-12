// -- Standard Library --
#include <stdexcept>

// -- Pompeii Includes --
#include "Context.h"
#include "CommandPool.h"
#include "Image.h"
#include "Buffer.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  CommandPool	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::CommandPool& pom::CommandPool::Create(const Context& context)
{
	pom::QueueFamilyIndices queueFamilyIndices = context.physicalDevice.GetQueueFamilies();

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(context.device.GetHandle(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Command Pool!");

	m_Device = context.device;
	return *this;
}
void pom::CommandPool::Destroy() const
{
	vkDestroyCommandPool(m_Device.GetHandle(), m_CommandPool, nullptr);
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
VkCommandPool& pom::CommandPool::GetHandle() { return m_CommandPool; }
pom::CommandBuffer& pom::CommandPool::GetBuffer(uint32_t bufferIdx)
{
	if (bufferIdx < 0 || bufferIdx >= m_vCommandBuffers.size())
		throw std::out_of_range("Buffer index out of range!");
	return m_vCommandBuffers[bufferIdx];
}
pom::CommandBuffer& pom::CommandPool::AllocateCmdBuffers(uint32_t count, VkCommandBufferLevel level)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.level = level;
	allocInfo.commandBufferCount = 1;

	for (uint32_t index{}; index < count; ++index)
	{
		VkCommandBuffer cmdBuffer;
		if (vkAllocateCommandBuffers(m_Device.GetHandle(), &allocInfo, &cmdBuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffer!");

		m_vCommandBuffers.emplace_back();
		m_vCommandBuffers.back().Initialize(m_CommandPool, cmdBuffer);
	}
	return m_vCommandBuffers.back();
}


//--------------------------------------------------
//    Command Helpers
//--------------------------------------------------
void pom::CommandPool::TransitionImageLayout(Image& image, VkImageLayout newLayout,
											uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount)
{
	VkImageLayout oldLayout = image.GetCurrentLayout();
	auto& cmd = AllocateCmdBuffers(1);
	cmd.Begin();
	{
		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.pNext = nullptr;

		barrier.image = image.GetHandle();
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstStageMask = 0;									// Gets Overwritten
		barrier.dstAccessMask = 0;									// Gets Overwritten

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcStageMask = 0;									// Gets Overwritten
		barrier.srcAccessMask = 0;									// Gets Overwritten

		barrier.subresourceRange.aspectMask = 0;					// Gets Overwritten
		barrier.subresourceRange.baseMipLevel = baseMip;
		barrier.subresourceRange.levelCount = mipCount;
		barrier.subresourceRange.baseArrayLayer = baseLayer;
		barrier.subresourceRange.layerCount = layerCount;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			barrier.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			barrier.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
			barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			barrier.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			barrier.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else throw std::invalid_argument("Unsupported Layout Transition!");

		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (image.HasStencilComponent()) barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;


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

		image.m_CurrentLayout = newLayout;
	}
	cmd.End();
	cmd.Submit(m_Device.GetGraphicQueue(), true);
	cmd.Free(m_Device);
}
void pom::CommandPool::CopyBufferToBuffer(const Buffer& srcBuffer, const Buffer& dstBuffer, VkDeviceSize size)
{
	auto& cmd = AllocateCmdBuffers(1);
	cmd.Begin();
	{
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(cmd.GetHandle(), srcBuffer.GetHandle(), dstBuffer.GetHandle(), 1, &copyRegion);
	}
	cmd.End();
	cmd.Submit(m_Device.GetGraphicQueue(), true);
	cmd.Free(m_Device);
}
void pom::CommandPool::CopyBufferToImage(const Buffer& buffer, const Image& image, VkExtent3D extent,
										uint32_t mip, uint32_t baseLayer, uint32_t layerCount)
{
	auto& cmd = AllocateCmdBuffers(1);
	cmd.Begin();
	{
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = mip;
		region.imageSubresource.baseArrayLayer = baseLayer;
		region.imageSubresource.layerCount = layerCount;
		region.imageOffset = { .x = 0, .y = 0, .z = 0};
		region.imageExtent = extent;

		vkCmdCopyBufferToImage(cmd.GetHandle(), buffer.GetHandle(), image.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}
	cmd.End();
	cmd.Submit(m_Device.GetGraphicQueue(), true);
	cmd.Free(m_Device);
}
