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
pompeii::CommandBuffer::CommandBuffer(VkCommandPool pool, VkCommandBuffer buffer)
	: m_CmdBuffer(buffer)
	, m_PoolOwner(pool)
{}

pompeii::CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
{
	m_CmdBuffer = std::move(other.m_CmdBuffer);
	other.m_CmdBuffer = VK_NULL_HANDLE;
	m_PoolOwner = std::move(other.m_PoolOwner);
	other.m_PoolOwner = VK_NULL_HANDLE;
}
pompeii::CommandBuffer& pompeii::CommandBuffer::operator=(CommandBuffer&& other) noexcept
{
	if (this == &other)
		return *this;
	m_CmdBuffer = std::move(other.m_CmdBuffer);
	other.m_CmdBuffer = VK_NULL_HANDLE;
	m_PoolOwner = std::move(other.m_PoolOwner);
	other.m_PoolOwner = VK_NULL_HANDLE;
	return *this;
}

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkCommandBuffer& pompeii::CommandBuffer::GetHandle() const { return m_CmdBuffer; }


//--------------------------------------------------
//    Commands
//--------------------------------------------------
void pompeii::CommandBuffer::Begin(VkCommandBufferUsageFlags usage) const
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = usage;

	if (vkBeginCommandBuffer(m_CmdBuffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin recording Command Buffer!");
}
void pompeii::CommandBuffer::End() const
{
	if (vkEndCommandBuffer(m_CmdBuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to record command buffer!");
}
void pompeii::CommandBuffer::Submit(VkQueue queue, bool waitIdle, const SemaphoreInfo& semaphoreInfo, VkFence fence) const
{
	// -- Submit Info --
	VkSubmitInfo2 submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submitInfo.pNext = nullptr;
	submitInfo.flags = 0;

	// -- Command Buffer Submit Info --
	VkCommandBufferSubmitInfo cmdSubmitInfo{};
	cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	cmdSubmitInfo.pNext = nullptr;
	cmdSubmitInfo.commandBuffer = m_CmdBuffer;
	cmdSubmitInfo.deviceMask = 0;
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &cmdSubmitInfo;

	// -- Wait Semaphores --
	std::vector<VkSemaphoreSubmitInfo> vWaitSemaphoreSubmitInfos;
	vWaitSemaphoreSubmitInfos.reserve(semaphoreInfo.vSignalSemaphores.size());
	if (!semaphoreInfo.vWaitSemaphores.empty())
	{
		uint32_t index = 0;
		for (const VkSemaphore& semaphore : semaphoreInfo.vWaitSemaphores)
		{
			VkSemaphoreSubmitInfo semaphoreSubmitInfo{};
			semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
			semaphoreSubmitInfo.pNext = nullptr;
			semaphoreSubmitInfo.value = 0;
			semaphoreSubmitInfo.semaphore = semaphore;
			semaphoreSubmitInfo.stageMask = semaphoreInfo.vWaitStages[index];

			vWaitSemaphoreSubmitInfos.emplace_back(semaphoreSubmitInfo);
			++index;
		}
		submitInfo.waitSemaphoreInfoCount = static_cast<uint32_t>(vWaitSemaphoreSubmitInfos.size());
		submitInfo.pWaitSemaphoreInfos = vWaitSemaphoreSubmitInfos.data();
	}

	// -- Signal Semaphores --
	std::vector<VkSemaphoreSubmitInfo> vSignalSemaphoreSubmitInfos;
	vSignalSemaphoreSubmitInfos.reserve(semaphoreInfo.vSignalSemaphores.size());
	if (!semaphoreInfo.vSignalSemaphores.empty())
	{
		for (const VkSemaphore& semaphore : semaphoreInfo.vSignalSemaphores)
		{
			VkSemaphoreSubmitInfo semaphoreSubmitInfo{};
			semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
			semaphoreSubmitInfo.pNext = nullptr;
			semaphoreSubmitInfo.value = 0;
			semaphoreSubmitInfo.semaphore = semaphore;

			vSignalSemaphoreSubmitInfos.emplace_back(semaphoreSubmitInfo);
		}
		submitInfo.signalSemaphoreInfoCount = static_cast<uint32_t>(vSignalSemaphoreSubmitInfos.size());
		submitInfo.pSignalSemaphoreInfos = vSignalSemaphoreSubmitInfos.data();
	}

	// -- Submit yay --
	if (vkQueueSubmit2(queue, 1, &submitInfo, fence) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit Command Buffer!");

	if (waitIdle)
		vkQueueWaitIdle(queue);
}
void pompeii::CommandBuffer::Reset() const { vkResetCommandBuffer(m_CmdBuffer, 0); }
void pompeii::CommandBuffer::Free(const Device& device) { vkFreeCommandBuffers(device.GetHandle(), m_PoolOwner, 1, &m_CmdBuffer); m_CmdBuffer = VK_NULL_HANDLE; }
