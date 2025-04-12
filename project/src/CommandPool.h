#ifndef COMMAND_POOL_H
#define COMMAND_POOL_H

// -- Vulkan Includes --
#include "vulkan/vulkan.h"

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "Device.h"
#include "CommandBuffer.h"

// -- Forward Declarations --
namespace pom
{
	class Buffer;
	class Image;
	struct Context;
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
		CommandPool& Create(const Context& context);
		void Destroy() const;


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		VkCommandPool& GetHandle();
		CommandBuffer& GetBuffer(uint32_t bufferIdx);
		CommandBuffer& AllocateCmdBuffers(uint32_t count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);


		//--------------------------------------------------
		//    Command Helpers
		//--------------------------------------------------
		void TransitionImageLayout(Image& image, VkImageLayout newLayout,
									uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount);
		void CopyBufferToBuffer(const Buffer& srcBuffer, const Buffer& dstBuffer, VkDeviceSize size);
		void CopyBufferToImage(const Buffer& buffer, const Image& image, VkExtent3D extent,
								uint32_t mip, uint32_t baseLayer, uint32_t layerCount);

	private:
		VkCommandPool				m_CommandPool		{ VK_NULL_HANDLE};
		std::vector<CommandBuffer>	m_vCommandBuffers	{ };
		Device						m_Device			{ };
	};
}

#endif // COMMAND_POOL_H