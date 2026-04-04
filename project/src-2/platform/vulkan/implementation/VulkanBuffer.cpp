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
	if (m_Buffer && m_Memory)
		vmaDestroyBuffer(m_pContext->allocator, m_Buffer, m_Memory);
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
size_t VulkanBuffer::GetSize()				const { return m_BufferDesc.size; }
BufferUsage VulkanBuffer::GetUsage()		const { return m_BufferDesc.usage; }
MemoryType VulkanBuffer::GetMemoryType()	const { return m_BufferDesc.memory; }


//--------------------------------------------------
//    Operator Overloads
//--------------------------------------------------
VulkanBuffer::operator VkBuffer()			const { return m_Buffer; }
VulkanBuffer::operator VmaAllocation()		const { return m_Memory; }

