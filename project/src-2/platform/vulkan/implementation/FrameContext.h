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
	//? ~~    FrameContext
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct FrameContext final : public IFrameContext
	{
	public:
		explicit FrameContext() = default;
		explicit FrameContext(VulkanContext& context);
		~FrameContext() override;
		FrameContext(FrameContext&&) noexcept = default;
		FrameContext& operator=(FrameContext&&) noexcept = default;

		VkSemaphore ImageAvailableSemaphore { VK_NULL_HANDLE };
		VkSemaphore RenderFinishedSemaphore { VK_NULL_HANDLE };
		VkFence InFlightFences				{ VK_NULL_HANDLE };

	private:
		VulkanContext* m_pContext{ nullptr };
	};
}

#endif // POMPEII_FRAME_CONTEXT_VULKAN_IMPLEMENTATION_H
