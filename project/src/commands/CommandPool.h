#ifndef COMMAND_POOL_H
#define COMMAND_POOL_H

// -- Vulkan Includes --
#include "vulkan/vulkan.h"

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "CommandBuffer.h"

// -- Forward Declarations --
namespace pom
{
	struct Context;
	class Buffer;
	class Image;
}


namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  CommandPool	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class CommandPool final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		CommandPool() = default;
		CommandPool& Create(Context& context);
		void Destroy() const;


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		VkCommandPool& GetHandle();
		CommandBuffer& GetBuffer(uint32_t bufferIdx);
		CommandBuffer& AllocateCmdBuffers(uint32_t count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	private:
		VkCommandPool				m_CommandPool		{ VK_NULL_HANDLE};
		std::vector<CommandBuffer>	m_vCommandBuffers	{ };
		Context*					m_Context			{ };
	};
}

#endif // COMMAND_POOL_H