#ifndef BUFFER_H
#define BUFFER_H

// -- Vulkan Includes --
#include <vma/vk_mem_alloc.h>

// -- Forward Declarations --
namespace pom
{
	class CommandPool;
	struct Context;
}


namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Buffer	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Buffer final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		Buffer() = default;
		void Destroy(const Context& context) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkBuffer& GetHandle() const;
		const VmaAllocation& GetMemoryHandle() const;

	private:
		VmaAllocation m_Memory;
		VkBuffer m_Buffer;

		friend class BufferAllocator;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Buffer Allocator	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class BufferAllocator final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		BufferAllocator();

		//--------------------------------------------------
		//    Allocator
		//--------------------------------------------------
		BufferAllocator& SetDebugName(const char* name);
		BufferAllocator& SetSize(uint32_t size);
		BufferAllocator& SetUsage(VkBufferUsageFlags usage);
		BufferAllocator& SetMemUsage(VmaMemoryUsage usage);
		BufferAllocator& SetSharingMode(VkSharingMode sharingMode);
		BufferAllocator& HostAccess(bool access);
		BufferAllocator& InitialData(void* data, uint32_t offset, uint32_t size, CommandPool& cmdPool);

		void Allocate(const Context& context, Buffer& buffer) const;

	private:
		bool m_UseInitialData;
		void* m_pData;
		uint32_t m_InitDataSize;
		uint32_t m_InitDataOffset;
		CommandPool* m_pCmdPool;

		const char* m_pName{};

		VkBufferCreateInfo m_CreateInfo{};
		VmaAllocationCreateInfo m_AllocCreateInfo{};
	};
}

#endif // BUFFER_H