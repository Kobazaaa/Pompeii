// -- Pompeii Includes --
#include "VulkanCommandPool.h"
#include "VulkanCommandBuffer.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"

using namespace pompeii;
using namespace pompeii::vulkan;

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
VulkanCommandPool::VulkanCommandPool(VulkanContext& context)
	: m_pContext(&context)
{
	const QueueFamilyIndices queueFamilyIndices = context.physicalDevice.GetQueueFamilies();

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(context.device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Command Pool!");
}


//--------------------------------------------------
//    Functionality
//--------------------------------------------------
std::unique_ptr<ICommandBuffer> VulkanCommandPool::Allocate(CommandBufferLevel level)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.level = TO_VK_LEVEL(level);
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer cmdBuffer;
	if (vkAllocateCommandBuffers(m_pContext->device, &allocInfo, &cmdBuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate command buffer!");

	auto ptr = std::make_unique<VulkanCommandBuffer>(*this, *m_pContext);
	ptr->m_CmdBuffer = cmdBuffer;
	return std::move(ptr);
}
void VulkanCommandPool::Free(ICommandBuffer* pCmdBuffer)
{
	const auto& cmdBufferHandle = static_cast<VulkanCommandBuffer*>(pCmdBuffer)->m_CmdBuffer;
	vkFreeCommandBuffers(m_pContext->device, m_CommandPool, 1, &cmdBufferHandle);
}
void VulkanCommandPool::Reset()
{
	vkResetCommandPool(m_pContext->device, m_CommandPool, 0);
}


//--------------------------------------------------
//    Functionality
//--------------------------------------------------
VulkanCommandPool::operator VkCommandPool() const { return m_CommandPool; }
