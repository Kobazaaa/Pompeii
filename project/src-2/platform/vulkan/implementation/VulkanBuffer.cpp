// -- Pompeii Includes --
#include "VulkanBuffer.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"

using namespace pompeii;
using namespace pompeii::vulkan;

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
VulkanBuffer::VulkanBuffer(const BufferDesc& bufferDesc, VulkanContext& context)
	: m_pContext(&context)
	, m_BufferDesc(bufferDesc)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = bufferDesc.size;
	bufferInfo.usage = TO_VK_USAGE(bufferDesc.usage);
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.requiredFlags = TO_VK_MEMORY(bufferDesc.memory);

	vmaCreateBuffer(
		context.allocator,
		&bufferInfo,
		&allocInfo,
		&m_Buffer,
		&m_Memory,
		nullptr
	);
}

VulkanBuffer::~VulkanBuffer()
{
	throw std::logic_error("Not implemented");
}


//--------------------------------------------------
//    Functionality
//--------------------------------------------------
void VulkanBuffer::SetData(const void* data, size_t size, size_t offset)
{
	throw std::logic_error("Not implemented");
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
size_t VulkanBuffer::GetSize()				const { return m_BufferDesc.size; }
BufferUsage VulkanBuffer::GetUsage()		const { return m_BufferDesc.usage; }
MemoryType VulkanBuffer::GetMemoryType()	const { return m_BufferDesc.memory; }

VkBuffer VulkanBuffer::GetHandle()			const { return m_Buffer; }


//--------------------------------------------------
//    Debug
//--------------------------------------------------
void VulkanBuffer::SetDebugName(const char* name)
{
	throw std::logic_error("Not implemented");
}
