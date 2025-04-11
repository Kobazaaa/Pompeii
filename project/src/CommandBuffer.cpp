// -- Standard Library --
#include <stdexcept>

// -- Pompeii Includes --
#include "CommandBuffer.h"
#include "Device.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  CommandBuffer	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::CommandBuffer::Initialize(VkCommandPool pool, VkCommandBuffer buffer)
{
	m_PoolOwner = pool;
	m_CmdBuffer = buffer;
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
VkCommandBuffer& pom::CommandBuffer::GetHandle() { return m_CmdBuffer; }


//--------------------------------------------------
//    Commands
//--------------------------------------------------
void pom::CommandBuffer::Begin(VkCommandBufferUsageFlags usage) const
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = usage;

	if (vkBeginCommandBuffer(m_CmdBuffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin recording Command Buffer!");
}
void pom::CommandBuffer::End() const
{
	if (vkEndCommandBuffer(m_CmdBuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to record command buffer!");
}
void pom::CommandBuffer::Submit(VkQueue queue, bool waitIdle, const SemaphoreInfo& semaphoreInfo, VkFence fence) const
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// Wait Semaphores
	if (!semaphoreInfo.vWaitSemaphores.empty())
	{
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(semaphoreInfo.vWaitSemaphores.size());
		submitInfo.pWaitSemaphores = semaphoreInfo.vWaitSemaphores.data();
		submitInfo.pWaitDstStageMask = semaphoreInfo.vWaitStages.data();
	}
	// Cmd Buffers
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CmdBuffer;

	// Signal Semaphores
	if (!semaphoreInfo.vSignalSemaphores.empty())
	{
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(semaphoreInfo.vSignalSemaphores.size());
		submitInfo.pSignalSemaphores = semaphoreInfo.vSignalSemaphores.data();
	}

	if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit Command Buffer!");


	if (waitIdle)
		vkQueueWaitIdle(queue);
}
void pom::CommandBuffer::Reset() const { vkResetCommandBuffer(m_CmdBuffer, 0); }
void pom::CommandBuffer::Free(const Device& device) const { vkFreeCommandBuffers(device.GetHandle(), m_PoolOwner, 1, &m_CmdBuffer); }
