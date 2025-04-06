#ifndef BUFFER_H
#define BUFFER_H

#include <vma/vk_mem_alloc.h>
#include "Device.h"

namespace pom
{
	class CommandPool;
	class Buffer final
	{
	public:
		Buffer() = default;
		void Destroy(const Device& device, const VmaAllocator& allocator) const;

		const VkBuffer& GetBuffer() const;
		const VmaAllocation& GetMemory() const;
	private:
		VmaAllocation m_Memory;
		VkBuffer m_Buffer;
		
		friend class BufferAllocator;
	};

	class BufferAllocator final
	{
	public:
		BufferAllocator();

		BufferAllocator& SetSize(uint32_t size);
		BufferAllocator& SetUsage(VkBufferUsageFlags usage);
		BufferAllocator& SetMemUsage(VmaMemoryUsage usage);
		BufferAllocator& SetSharingMode(VkSharingMode sharingMode);
		BufferAllocator& HostAccess(bool access);
		BufferAllocator& InitialData(void* data, uint32_t offset, uint32_t size);

		void Allocate(const Device& device, const VmaAllocator& allocator, CommandPool& cmdPool, Buffer& buffer);

	private:
		bool m_UseInitialData;
		void* m_pData;
		uint32_t m_InitDataSize;
		uint32_t m_InitDataOffset;

		VkBufferCreateInfo m_CreateInfo{};
		VmaAllocationCreateInfo m_AllocCreateInfo{};
	};
}

#endif // BUFFER_H