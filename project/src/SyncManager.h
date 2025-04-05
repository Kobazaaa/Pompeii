#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

#include "Device.h"

namespace pom
{
	struct SemaphoreInfo
	{
		std::vector<VkSemaphore>			vWaitSemaphores;
		std::vector < VkPipelineStageFlags> vWaitStages;
		std::vector<VkSemaphore>			vSignalSemaphores;
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  SyncManager	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class SyncManager final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		SyncManager() = default;
		void Create(const Device& device);
		void Cleanup();

		//--------------------------------------------------
		//    Makers
		//--------------------------------------------------
		VkSemaphore CreateSemaphore();
		VkFence CreateFence(bool signaled);

	private:
		Device						m_Device		{ VK_NULL_HANDLE };
		std::vector<VkSemaphore>	m_vSemaphores	{};
		std::vector<VkFence>		m_vFences		{};

	};
}

#endif // SYNC_MANAGER_H