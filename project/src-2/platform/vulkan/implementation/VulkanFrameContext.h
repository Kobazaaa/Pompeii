#ifndef POMPEII_FRAME_CONTEXT_VULKAN_IMPLEMENTATION_H
#define POMPEII_FRAME_CONTEXT_VULKAN_IMPLEMENTATION_H

// -- Pompeii Includes --
#include "IFrameContext.h"

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
	//? ~~    VulkanFrameContext
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct VulkanFrameContext final : public IFrameContext
	{
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit VulkanFrameContext() = default;
		explicit VulkanFrameContext(VulkanContext& context);
		~VulkanFrameContext() override;

		VulkanFrameContext(const VulkanFrameContext& other) = delete;
		VulkanFrameContext(VulkanFrameContext&& other) noexcept = default;
		VulkanFrameContext& operator=(const VulkanFrameContext& other) = delete;
		VulkanFrameContext& operator=(VulkanFrameContext&& other) noexcept = default;

		//--------------------------------------------------
		//    Data
		//--------------------------------------------------
		VkSemaphore ImageAvailableSemaphore { VK_NULL_HANDLE };
		VkSemaphore RenderFinishedSemaphore { VK_NULL_HANDLE };
		VkFence InFlightFences				{ VK_NULL_HANDLE };

	private:
		VulkanContext* m_pContext{ nullptr };
	};
}

#endif // POMPEII_FRAME_CONTEXT_VULKAN_IMPLEMENTATION_H
