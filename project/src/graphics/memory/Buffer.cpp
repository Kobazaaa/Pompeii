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
void pom::Buffer::Destroy(const Context& context) const
{
	vmaDestroyBuffer(context.allocator, m_Buffer, m_Memory);
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkBuffer& pom::Buffer::GetHandle() const { return m_Buffer; }
const VmaAllocation& pom::Buffer::GetMemoryHandle() const { return m_Memory; }


//--------------------------------------------------
//    Commands
//--------------------------------------------------
void pom::Buffer::CopyToBuffer(const CommandBuffer& cmd, const Buffer& dst, VkDeviceSize size) const
{
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(cmd.GetHandle(), m_Buffer, dst.GetHandle(), 1, &copyRegion);
}
void pom::Buffer::CopyToImage(const CommandBuffer& cmd, const Image& dst, VkExtent3D extent, uint32_t mip, uint32_t baseLayer, uint32_t layerCount) const
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
pom::BufferAllocator::BufferAllocator()
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
	m_pData = nullptr;														//? CAN CHANGE
	m_InitDataSize = 0;														//? CAN CHANGE
	m_InitDataOffset = 0;													//? CAN CHANGE
	m_pName = nullptr;														//? CAN CHANGE
	m_pCmdPool = nullptr;													//? CAN CHANGE
}



//--------------------------------------------------
//    Allocator
//--------------------------------------------------
pom::BufferAllocator& pom::BufferAllocator::SetDebugName(const char* name)
{
	m_pName = name;
	return *this;
}
pom::BufferAllocator& pom::BufferAllocator::SetSize(uint32_t size)
{
	m_CreateInfo.size = size;
	return *this;
}
pom::BufferAllocator& pom::BufferAllocator::SetUsage(VkBufferUsageFlags usage)
{
	m_CreateInfo.usage = usage;
	return *this;
}
pom::BufferAllocator& pom::BufferAllocator::SetMemUsage(VmaMemoryUsage usage)
{
	m_AllocCreateInfo.usage = usage;
	return *this;
}
pom::BufferAllocator& pom::BufferAllocator::SetSharingMode(VkSharingMode sharingMode)
{
	m_CreateInfo.usage = sharingMode;
	return *this;
}
pom::BufferAllocator& pom::BufferAllocator::HostAccess(bool access)
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
pom::BufferAllocator& pom::BufferAllocator::InitialData(void* data, uint32_t offset, uint32_t size, CommandPool& cmdPool)
{
	m_UseInitialData = true;
	m_pData = data;
	m_InitDataOffset = offset;
	m_InitDataSize = size;
	m_CreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	m_pCmdPool = &cmdPool;
	return *this;
}

void pom::BufferAllocator::Allocate(const Context& context, Buffer& buffer) const
{
	vmaCreateBuffer(context.allocator, &m_CreateInfo, &m_AllocCreateInfo, &buffer.m_Buffer, &buffer.m_Memory, nullptr);

	if (m_UseInitialData)
	{
		Buffer stagingBuffer;
		BufferAllocator stagingAllocator{};
		stagingAllocator
			.SetUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			.HostAccess(true)
			.SetSize(m_InitDataSize)
			.Allocate(context, stagingBuffer);
		vmaCopyMemoryToAllocation(context.allocator, m_pData, stagingBuffer.m_Memory, m_InitDataOffset, m_InitDataSize);


		CommandBuffer& cmd = m_pCmdPool->AllocateCmdBuffers(1);
		cmd.Begin();
		{
			stagingBuffer.CopyToBuffer(cmd, buffer, m_InitDataSize);
		}
		cmd.End();
		cmd.Submit(context.device.GetGraphicQueue(), true);
		cmd.Free(context.device);
		stagingBuffer.Destroy(context);
	}
	if (m_pName)
	{
		Debugger::SetDebugObjectName(reinterpret_cast<uint64_t>(buffer.GetHandle()), VK_OBJECT_TYPE_BUFFER, m_pName);
	}
}
