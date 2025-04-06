#include "SyncManager.h"
#include <stdexcept>

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  SyncManager	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::SyncManager::Create(const Device& device, uint32_t maxFramesInFlight)
{
	m_MaxFrames = maxFramesInFlight;
	m_Device = device;

	m_vFrameSyncs.resize(maxFramesInFlight);
	for (uint32_t index{}; index < maxFramesInFlight; ++index)
	{
		m_vFrameSyncs[index].imageAvailable = CreateSemaphore();
		m_vFrameSyncs[index].renderFinished = CreateSemaphore();
		m_vFrameSyncs[index].inFlight = CreateFence(true);
	}
}
void pom::SyncManager::Cleanup()
{
	for (auto& semaphore : m_vSemaphores)
		vkDestroySemaphore(m_Device.GetDevice(), semaphore, nullptr);

	for (auto& fence : m_vFences)
		vkDestroyFence(m_Device.GetDevice(), fence, nullptr);

	m_vFrameSyncs.clear();
	m_vSemaphores.clear();
	m_vFences.clear();
}

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const pom::FrameSync& pom::SyncManager::GetFrameSync(uint32_t frame) const
{
	return m_vFrameSyncs[frame % m_MaxFrames];
}


//--------------------------------------------------
//    Makers
//--------------------------------------------------
VkSemaphore pom::SyncManager::CreateSemaphore()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkSemaphore semaphore;
	if (vkCreateSemaphore(m_Device.GetDevice(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
		throw std::runtime_error("Failed to create semaphore!");

	m_vSemaphores.push_back(semaphore);
	return semaphore;
}
VkFence pom::SyncManager::CreateFence(bool signaled)
{
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

	VkFence fence;
	if (vkCreateFence(m_Device.GetDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS)
		throw std::runtime_error("Failed to create fence!");

	m_vFences.push_back(fence);
	return fence;
}
