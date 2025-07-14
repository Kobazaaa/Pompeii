// -- Pompeii Includes --
#include "Buffer.h"
#include "CommandPool.h"
#include "Context.h"
#include "Image.h"
#include "Debugger.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Buffer	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::Buffer::Buffer(Buffer&& other) noexcept
{
	m_Memory = std::move(other.m_Memory);
	other.m_Memory = VK_NULL_HANDLE;
	m_Buffer = std::move(other.m_Buffer);
	other.m_Buffer = VK_NULL_HANDLE;
}
pompeii::Buffer& pompeii::Buffer::operator=(Buffer&& other) noexcept
{
	if (this == &other)
		return *this;
	m_Memory = std::move(other.m_Memory);
	other.m_Memory = VK_NULL_HANDLE;
	m_Buffer = std::move(other.m_Buffer);
	other.m_Buffer = VK_NULL_HANDLE;
	return *this;
}

void pompeii::Buffer::Destroy(const Context& context) const
{
	vmaDestroyBuffer(context.allocator, m_Buffer, m_Memory);
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkBuffer& pompeii::Buffer::GetHandle() const { return m_Buffer; }
const VmaAllocation& pompeii::Buffer::GetMemoryHandle() const { return m_Memory; }
VkDeviceSize pompeii::Buffer::Size() const { return m_Size; }

//--------------------------------------------------
//    Commands
//--------------------------------------------------
void pompeii::Buffer::InsertBarrier(const CommandBuffer& cmd,
								VkAccessFlags2 srcAccess, VkPipelineStageFlags2 srcStage,
								VkAccessFlags2 dstAccess, VkPipelineStageFlags2 dstStage) const
{
	VkBufferMemoryBarrier2 barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
	barrier.pNext = nullptr;

	barrier.buffer = m_Buffer;
	barrier.size = VK_WHOLE_SIZE;
	barrier.offset = 0;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.srcAccessMask = srcAccess;
	barrier.srcStageMask = srcStage;

	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstAccessMask = dstAccess;
	barrier.dstStageMask = dstStage;

	VkDependencyInfo dependencyInfo{};
	dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependencyInfo.dependencyFlags = 0;
	dependencyInfo.pNext = nullptr;
	dependencyInfo.memoryBarrierCount = 0;
	dependencyInfo.pMemoryBarriers = nullptr;
	dependencyInfo.bufferMemoryBarrierCount = 1;
	dependencyInfo.pBufferMemoryBarriers = &barrier;
	dependencyInfo.imageMemoryBarrierCount = 0;
	dependencyInfo.pImageMemoryBarriers = nullptr;

	vkCmdPipelineBarrier2(cmd.GetHandle(), &dependencyInfo);
}
void pompeii::Buffer::CopyToBuffer(const CommandBuffer& cmd, const Buffer& dst, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset) const
{
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = srcOffset;
	copyRegion.dstOffset = dstOffset;
	copyRegion.size = size;
	vkCmdCopyBuffer(cmd.GetHandle(), m_Buffer, dst.GetHandle(), 1, &copyRegion);
}
void pompeii::Buffer::CopyToImage(const CommandBuffer& cmd, const Image& dst, VkExtent3D extent, uint32_t mip, uint32_t baseLayer, uint32_t layerCount) const
{
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = mip;
	region.imageSubresource.baseArrayLayer = baseLayer;
	region.imageSubresource.layerCount = layerCount;
	region.imageOffset = { .x = 0, .y = 0, .z = 0 };
	region.imageExtent = extent;

	vkCmdCopyBufferToImage(cmd.GetHandle(), m_Buffer, dst.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Buffer Allocator	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::BufferAllocator::BufferAllocator()
{
	m_CreateInfo = {};
	m_CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;				// CAN'T CHANGE
	m_CreateInfo.size = 0;													//! REQUIRED CHANGE
	m_CreateInfo.usage = 0;													//! REQUIRED CHANGE
	m_CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;					//? CAN CHANGE

	m_AllocCreateInfo = {};
	m_AllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;						//? CAN CHANGE
	m_AllocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;	//? CAN CHANGE

	m_UseInitialData = false;												//? CAN CHANGE
	m_vInitialData.clear();													//? CAN CHANGE
	m_pName = nullptr;														//? CAN CHANGE
}



//--------------------------------------------------
//    Allocator
//--------------------------------------------------
pompeii::BufferAllocator& pompeii::BufferAllocator::SetDebugName(const char* name)
{
	m_pName = name;
	return *this;
}
pompeii::BufferAllocator& pompeii::BufferAllocator::SetSize(uint32_t size)
{
	m_CreateInfo.size = size;
	return *this;
}
pompeii::BufferAllocator& pompeii::BufferAllocator::SetUsage(VkBufferUsageFlags usage)
{
	m_CreateInfo.usage = usage;
	return *this;
}
pompeii::BufferAllocator& pompeii::BufferAllocator::SetMemUsage(VmaMemoryUsage usage)
{
	m_AllocCreateInfo.usage = usage;
	return *this;
}
pompeii::BufferAllocator& pompeii::BufferAllocator::SetSharingMode(VkSharingMode sharingMode)
{
	m_CreateInfo.usage = sharingMode;
	return *this;
}
pompeii::BufferAllocator& pompeii::BufferAllocator::HostAccess(bool access)
{
	if (access)
	{
		m_AllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
		m_AllocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	}
	else
	{
		m_AllocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}

	return *this;
}
pompeii::BufferAllocator& pompeii::BufferAllocator::AddInitialData(void* data, VkDeviceSize dstOffset, uint32_t size)
{
	m_UseInitialData = true;
	m_vInitialData.emplace_back(data, size, dstOffset);
	m_CreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	return *this;
}

void pompeii::BufferAllocator::Allocate(const Context& context, Buffer& buffer) const
{
	vmaCreateBuffer(context.allocator, &m_CreateInfo, &m_AllocCreateInfo, &buffer.m_Buffer, &buffer.m_Memory, nullptr);
	buffer.m_Size = m_CreateInfo.size;

	if (m_UseInitialData)
	{
		for (const InitData& data : m_vInitialData)
		{
			if (data.initDataSize <= 0)
				continue;

			Buffer stagingBuffer;
			BufferAllocator stagingAllocator{};
			stagingAllocator
				.SetUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
				.HostAccess(true)
				.SetSize(data.initDataSize)
				.Allocate(context, stagingBuffer);
			vmaCopyMemoryToAllocation(context.allocator, data.pData, stagingBuffer.m_Memory, 0, data.initDataSize);


			CommandBuffer& cmd = context.commandPool->AllocateCmdBuffers(1);
			cmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			{
				stagingBuffer.CopyToBuffer(cmd, buffer, data.initDataSize, 0, data.dstOffset);
			}
			cmd.End();
			cmd.Submit(context.device.GetGraphicQueue(), true);
			cmd.Free(context.device);
			stagingBuffer.Destroy(context);
		}
	}
	if (m_pName)
	{
		Debugger::SetDebugObjectName(reinterpret_cast<uint64_t>(buffer.GetHandle()), VK_OBJECT_TYPE_BUFFER, m_pName);
	}
}
