#ifndef BUFFER_H
#define BUFFER_H

// -- Vulkan Includes --
#include <vector>
#include <vma/vk_mem_alloc.h>

// -- Forward Declarations --
namespace pom
{
	class CommandBuffer;
	class Image;
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
		explicit Buffer() = default;
		~Buffer() = default;
		Buffer(const Buffer& other) = delete;
		Buffer(Buffer&& other) noexcept;
		Buffer& operator=(const Buffer& other) = delete;
		Buffer& operator=(Buffer&& other) noexcept;
		void Destroy(const Context& context) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkBuffer& GetHandle() const;
		const VmaAllocation& GetMemoryHandle() const;
		VkDeviceSize Size() const;

		//--------------------------------------------------
		//    Commands
		//--------------------------------------------------
		void InsertBarrier(const CommandBuffer& cmd, VkAccessFlags2 srcAccess, VkPipelineStageFlags2 srcStage, VkAccessFlags2 dstAccess, VkPipelineStageFlags2 dstStage) const;
		void CopyToBuffer(const CommandBuffer& cmd, const Buffer& dst, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset) const;
		void CopyToImage(const CommandBuffer& cmd, const Image& dst, VkExtent3D extent, uint32_t mip, uint32_t baseLayer, uint32_t layerCount) const;

	private:
		VmaAllocation m_Memory;
		VkBuffer m_Buffer;
		VkDeviceSize m_Size;

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
		BufferAllocator& AddInitialData(void* data, VkDeviceSize dstOffset, uint32_t size, CommandPool& cmdPool);

		void Allocate(const Context& context, Buffer& buffer) const;

	private:
		bool m_UseInitialData;
		struct InitData
		{
			void* pData;
			uint32_t initDataSize;
			VkDeviceSize dstOffset;
			CommandPool* pCmdPool;
		};
		std::vector<InitData> m_vInitialData{};

		const char* m_pName{};

		VkBufferCreateInfo m_CreateInfo{};
		VmaAllocationCreateInfo m_AllocCreateInfo{};
	};
}

#endif // BUFFER_H