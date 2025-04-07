#ifndef COMMAND_POOL_H
#define COMMAND_POOL_H

#include "CommandBuffer.h"
#include "Buffer.h"
#include "Device.h"

namespace pom
{
	class Image;
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
		CommandPool& Create(Device& device, const PhysicalDevice& physicalDevice);
		void Destroy() const;


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		VkCommandPool& GetPool();
		CommandBuffer& GetBuffer(uint32_t bufferIdx);
		CommandBuffer& AllocateCmdBuffers(uint32_t count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);


		//--------------------------------------------------
		//    Command Helpers
		//--------------------------------------------------
		void TransitionImageLayout(Image& image, VkImageLayout newLayout);
		void CopyBufferToBuffer(const Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size);
		void CopyBufferToImage(const Buffer& buffer, const Image& image, uint32_t width, uint32_t height);

	private:
		VkCommandPool				m_CommandPool		{ VK_NULL_HANDLE};
		std::vector<CommandBuffer>	m_vCommandBuffers	{ };
		Device						m_Device			{ };
	};
}

#endif // COMMAND_POOL_H