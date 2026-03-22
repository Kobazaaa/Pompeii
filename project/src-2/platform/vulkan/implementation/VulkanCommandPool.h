#ifndef POMPEII_VULKAN_COMMAND_POOL_IMPLEMENTATION_H
#define POMPEII_VULKAN_COMMAND_POOL_IMPLEMENTATION_H

// -- Standard Library --
#include <memory>

// -- Pompeii Includes --
#include "ICommandPool.h"

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Forward Declarations --
namespace pompeii::vulkan
{
	struct VulkanContext;
}

namespace pompeii::vulkan
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    VulkanCommandPool
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class VulkanCommandPool final : public ICommandPool
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit VulkanCommandPool() = default;
		explicit VulkanCommandPool(VulkanContext& context);
		~VulkanCommandPool() override = default;

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		std::unique_ptr<ICommandBuffer> Allocate(CommandBufferLevel level) override;
		void Free(ICommandBuffer* pCmdBuffer) override;
		void Reset() override;

	private:
		VulkanContext* m_pContext	{ nullptr };
		VkCommandPool m_CommandPool	{ VK_NULL_HANDLE };
	};
}

#endif // POMPEII_VULKAN_COMMAND_POOL_IMPLEMENTATION_H
