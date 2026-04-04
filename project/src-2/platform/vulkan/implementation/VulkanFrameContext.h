#ifndef POMPEII_FRAME_CONTEXT_VULKAN_IMPLEMENTATION_H
#define POMPEII_FRAME_CONTEXT_VULKAN_IMPLEMENTATION_H

// -- Pompeii Includes --
#include "FrameContext.h"

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
	class VulkanFrameContext final : public FrameContext
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit VulkanFrameContext() = default;
		explicit VulkanFrameContext(VulkanContext& context, std::unique_ptr<ICommandBuffer> cmdBuffer, uint32_t index);
		~VulkanFrameContext() override;

		VulkanFrameContext(const VulkanFrameContext& other) = delete;
		VulkanFrameContext(VulkanFrameContext&& other) noexcept = default;
		VulkanFrameContext& operator=(const VulkanFrameContext& other) = delete;
		VulkanFrameContext& operator=(VulkanFrameContext&& other) noexcept = default;

		//--------------------------------------------------
		//    Data
		//--------------------------------------------------
		VkSemaphore GetImageAvailableSemaphore() const;
		VkSemaphore GetRenderFinishedSemaphore() const;
		VkFence GetInFlightFence() const;

	private:
		VkSemaphore m_ImageAvailableSemaphore	{ VK_NULL_HANDLE };
		VkSemaphore m_RenderFinishedSemaphore	{ VK_NULL_HANDLE };
		VkFence m_InFlightFences				{ VK_NULL_HANDLE };
		VulkanContext* m_pContext				{ nullptr };

	};
}

#endif // POMPEII_FRAME_CONTEXT_VULKAN_IMPLEMENTATION_H
