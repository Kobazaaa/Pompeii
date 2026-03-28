// -- Pompeii::vulkan Includes --
#include "FrameContext.h"
#include "VulkanContext.h"

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::vulkan::FrameContext::FrameContext(VulkanContext& context)
	: m_pContext(&context)
{
	VkSemaphoreCreateInfo semInfo
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0
	};
	VkFenceCreateInfo fenceInfo
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	vkCreateSemaphore(m_pContext->device, &semInfo, nullptr, &ImageAvailableSemaphore);
	vkCreateSemaphore(m_pContext->device, &semInfo, nullptr, &RenderFinishedSemaphore);
	vkCreateFence(m_pContext->device, &fenceInfo, nullptr, &InFlightFences);
}

pompeii::vulkan::FrameContext::~FrameContext()
{
	if (InFlightFences) vkDestroyFence(m_pContext->device, InFlightFences, nullptr);
	if (ImageAvailableSemaphore) vkDestroySemaphore(m_pContext->device, ImageAvailableSemaphore, nullptr);
	if (RenderFinishedSemaphore) vkDestroySemaphore(m_pContext->device, RenderFinishedSemaphore, nullptr);
}
