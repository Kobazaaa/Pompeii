#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Pompeii Includes --
#include "SyncManager.h"

// -- Forward Declarations --
namespace pom
{
	class Device;
	struct SemaphoreInfo;
}


namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  CommandBuffer	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class CommandBuffer final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		CommandBuffer() = default;
		void Initialize(VkCommandPool pool, VkCommandBuffer buffer);


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		VkCommandBuffer& GetHandle();


		//--------------------------------------------------
		//    Commands
		//--------------------------------------------------
		void Begin(VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) const;
		void End() const;
		void Submit(VkQueue queue, bool waitIdle, const SemaphoreInfo& semaphoreInfo = {}, VkFence fence = VK_NULL_HANDLE) const;
		void Reset() const;
		void Free(const Device& device) const;

	private:
		VkCommandBuffer m_CmdBuffer { VK_NULL_HANDLE };
		VkCommandPool	m_PoolOwner	{ VK_NULL_HANDLE };
	};
}

#endif // COMMAND_BUFFER_H