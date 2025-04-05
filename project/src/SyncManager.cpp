#include "SyncManager.h"
#include <stdexcept>

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  SyncManager	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::SyncManager::Create(const Device& device)
{
	m_Device = device;
}
void pom::SyncManager::Cleanup()
{
	for (auto& semaphore : m_vSemaphores)
		vkDestroySemaphore(m_Device.GetDevice(), semaphore, nullptr);

	for (auto& fence : m_vFences)
		vkDestroyFence(m_Device.GetDevice(), fence, nullptr);

	m_vSemaphores.clear();
	m_vFences.clear();
}


//--------------------------------------------------
//    Constructor & Destructor
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
