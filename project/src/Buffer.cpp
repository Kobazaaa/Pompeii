#include "Buffer.h"
#include "CommandPool.h"

void pom::Buffer::Destroy(const Device& device, const VmaAllocator& allocator) const
{
	vkDestroyBuffer(device.GetDevice(), m_Buffer, nullptr);
	vmaFreeMemory(allocator, m_Memory);
}

const VkBuffer& pom::Buffer::GetBuffer() const
{
	return m_Buffer;
}

const VmaAllocation& pom::Buffer::GetMemory() const
{
	return m_Memory;
}

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

pom::BufferAllocator& pom::BufferAllocator::InitialData(void* data, uint32_t offset, uint32_t size)
{
	m_UseInitialData = true;
	m_pData = data;
	m_InitDataOffset = offset;
	m_InitDataSize = size;
	m_CreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	return *this;
}

void pom::BufferAllocator::Allocate(const Device& device, const VmaAllocator& allocator, CommandPool& cmdPool, Buffer& buffer) const
{
	vmaCreateBuffer(allocator, &m_CreateInfo, &m_AllocCreateInfo, &buffer.m_Buffer, &buffer.m_Memory, nullptr);

	if (m_UseInitialData)
	{
		Buffer stagingBuffer;
		BufferAllocator stagingAllocator{};
		stagingAllocator
			.SetUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			.HostAccess(true)
			.SetSize(m_InitDataSize)
			.Allocate(device, allocator, cmdPool, stagingBuffer);
		vmaCopyMemoryToAllocation(allocator, m_pData, stagingBuffer.m_Memory, m_InitDataOffset, m_InitDataSize);

		cmdPool.CopyBufferToBuffer(stagingBuffer, buffer, m_InitDataSize);

		stagingBuffer.Destroy(device, allocator);
	}
}
