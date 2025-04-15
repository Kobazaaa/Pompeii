#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

// -- Vulkan Includes --
#include "vulkan/vulkan.h"

// -- Standard Library --
#include <vector>

// -- Forward Declarations --
namespace pom { struct Context; }


// -- Class --
namespace pom
{
	// -- Helper Structs --
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
		void Create(const Context& context, uint32_t maxFramesInFlight);
		void Cleanup(const Context& context);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const FrameSync& GetFrameSync(uint32_t frame) const;

		//--------------------------------------------------
		//    Makers
		//--------------------------------------------------
		const VkSemaphore& CreateSemaphore(const Context& context);
		const VkFence& CreateFence(const Context& context, bool signaled);

	private:
		uint32_t					m_MaxFrames		{};

		std::vector<VkSemaphore>	m_vSemaphores	{};
		std::vector<VkFence>		m_vFences		{};

		std::vector<FrameSync>		m_vFrameSyncs	{};
	};
}

#endif // SYNC_MANAGER_H