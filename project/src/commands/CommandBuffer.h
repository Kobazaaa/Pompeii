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
		explicit CommandBuffer(VkCommandPool pool, VkCommandBuffer buffer);
		~CommandBuffer() = default;
		CommandBuffer(const CommandBuffer& other) = delete;
		CommandBuffer(CommandBuffer&& other) noexcept;
		CommandBuffer& operator=(const CommandBuffer& other) = delete;
		CommandBuffer& operator=(CommandBuffer&& other) noexcept;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkCommandBuffer& GetHandle() const;

		//--------------------------------------------------
		//    Commands
		//--------------------------------------------------
		void Begin(VkCommandBufferUsageFlags usage = 0) const;
		void End() const;
		void Submit(VkQueue queue, bool waitIdle, const SemaphoreInfo& semaphoreInfo = {}, VkFence fence = VK_NULL_HANDLE) const;
		void Reset() const;
		void Free(const Device& device);

	private:
		VkCommandBuffer m_CmdBuffer { VK_NULL_HANDLE };
		VkCommandPool	m_PoolOwner	{ VK_NULL_HANDLE };
	};
}

#endif // COMMAND_BUFFER_H