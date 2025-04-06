#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H
#include <vulkan/vulkan.h>

#include "Device.h"
#include "SyncManager.h"

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
		void Allocate(Device device, VkCommandPool pool, VkCommandBuffer buffer);


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		VkCommandBuffer& GetBuffer();


		//--------------------------------------------------
		//    Commands
		//--------------------------------------------------
		void Begin(VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) const;
		void End() const;
		void Submit(VkQueue queue, bool waitIdle, const SemaphoreInfo& semaphoreInfo = {}, VkFence fence = VK_NULL_HANDLE) const;
		void Reset() const;
		void Free();

	private:
		Device			m_Device	{};
		VkCommandBuffer m_CmdBuffer { VK_NULL_HANDLE };
		VkCommandPool	m_PoolOwner	{ VK_NULL_HANDLE };
	};
}

#endif // COMMAND_BUFFER_H