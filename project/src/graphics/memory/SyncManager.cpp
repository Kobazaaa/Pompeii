// -- Standard Library --
#include <stdexcept>

// -- Pompeii Includes --
#include "SyncManager.h"
#include "Context.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  SyncManager	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pompeii::SyncManager::Create(const Context& context, uint32_t maxFramesInFlight)
{
	m_MaxFrames = maxFramesInFlight;

	m_vFrameSyncs.resize(maxFramesInFlight);
	for (uint32_t index{}; index < maxFramesInFlight; ++index)
	{
		m_vFrameSyncs[index].imageAvailable = CreateSemaphore(context);
		m_vFrameSyncs[index].renderFinished = CreateSemaphore(context);
		m_vFrameSyncs[index].inFlight = CreateFence(context, true);
	}
}
void pompeii::SyncManager::Cleanup(const Context& context)
{
	for (auto& semaphore : m_vSemaphores)
		vkDestroySemaphore(context.device.GetHandle(), semaphore, nullptr);

	for (auto& fence : m_vFences)
		vkDestroyFence(context.device.GetHandle(), fence, nullptr);

	m_vFrameSyncs.clear();
	m_vSemaphores.clear();
	m_vFences.clear();
}

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const pompeii::FrameSync& pompeii::SyncManager::GetFrameSync(uint32_t frame) const { return m_vFrameSyncs[frame % m_MaxFrames]; }


//--------------------------------------------------
//    Makers
//--------------------------------------------------
const VkSemaphore& pompeii::SyncManager::CreateSemaphore(const Context& context)
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkSemaphore semaphore;
	if (vkCreateSemaphore(context.device.GetHandle(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
		throw std::runtime_error("Failed to create semaphore!");

	m_vSemaphores.push_back(semaphore);
	return m_vSemaphores.back();
}
const VkFence& pompeii::SyncManager::CreateFence(const Context& context, bool signaled)
{
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

	VkFence fence;
	if (vkCreateFence(context.device.GetHandle(), &fenceInfo, nullptr, &fence) != VK_SUCCESS)
		throw std::runtime_error("Failed to create fence!");

	m_vFences.push_back(fence);
	return m_vFences.back();
}
