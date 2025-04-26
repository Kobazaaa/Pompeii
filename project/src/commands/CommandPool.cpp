// -- Standard Library --
#include <stdexcept>

// -- Pompeii Includes --
#include "Context.h"
#include "CommandPool.h"
#include "Image.h"
#include "Buffer.h"
#include "Context.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  CommandPool	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::CommandPool& pom::CommandPool::Create(Context& context)
{
	pom::QueueFamilyIndices queueFamilyIndices = context.physicalDevice.GetQueueFamilies();

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(context.device.GetHandle(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Command Pool!");

	m_Context = &context;
	return *this;
}
void pom::CommandPool::Destroy() const
{
	vkDestroyCommandPool(m_Context->device.GetHandle(), m_CommandPool, nullptr);
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
		if (vkAllocateCommandBuffers(m_Context->device.GetHandle(), &allocInfo, &cmdBuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffer!");

		m_vCommandBuffers.emplace_back();
		m_vCommandBuffers.back().Initialize(m_CommandPool, cmdBuffer);
	}
	return m_vCommandBuffers.back();
}
