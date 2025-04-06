#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

#include "Device.h"

namespace pom
{
	struct FrameSync
	{
		VkSemaphore imageAvailable;
		VkSemaphore renderFinished;
		VkFence inFlight;
	};
	struct SemaphoreInfo
	{
		std::vector<VkSemaphore>			vWaitSemaphores;
		std::vector<VkPipelineStageFlags>	vWaitStages;
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
		void Create(const Device& device, uint32_t maxFramesInFlight);
		void Cleanup();

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const FrameSync& GetFrameSync(uint32_t frame) const;

		//--------------------------------------------------
		//    Makers
		//--------------------------------------------------
		VkSemaphore CreateSemaphore();
		VkFence CreateFence(bool signaled);

	private:
		uint32_t					m_MaxFrames		{};
		Device						m_Device		{};

		std::vector<VkSemaphore>	m_vSemaphores	{};
		std::vector<VkFence>		m_vFences		{};

		std::vector<FrameSync>		m_vFrameSyncs	{};
	};
}

#endif // SYNC_MANAGER_H