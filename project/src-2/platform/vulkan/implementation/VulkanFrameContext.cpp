// -- Pompeii Includes --
#include "VulkanFrameContext.h"
#include "VulkanContext.h"

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::vulkan::VulkanFrameContext::VulkanFrameContext(VulkanContext& context, std::unique_ptr<ICommandBuffer> cmdBuffer, uint32_t index)
	: FrameContext(std::move(cmdBuffer), index)
	, m_pContext(&context)
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

	vkCreateSemaphore(m_pContext->device, &semInfo, nullptr, &m_ImageAvailableSemaphore);
	vkCreateSemaphore(m_pContext->device, &semInfo, nullptr, &m_RenderFinishedSemaphore);
	vkCreateFence(m_pContext->device, &fenceInfo, nullptr, &m_InFlightFences);
}

pompeii::vulkan::VulkanFrameContext::~VulkanFrameContext()
{
	if (m_InFlightFences) vkDestroyFence(m_pContext->device, m_InFlightFences, nullptr);
	if (m_ImageAvailableSemaphore) vkDestroySemaphore(m_pContext->device, m_ImageAvailableSemaphore, nullptr);
	if (m_RenderFinishedSemaphore) vkDestroySemaphore(m_pContext->device, m_RenderFinishedSemaphore, nullptr);
}


//--------------------------------------------------
//    Data
//--------------------------------------------------
VkSemaphore pompeii::vulkan::VulkanFrameContext::GetImageAvailableSemaphore()	const { return m_ImageAvailableSemaphore; }
VkSemaphore pompeii::vulkan::VulkanFrameContext::GetRenderFinishedSemaphore()	const { return m_RenderFinishedSemaphore; }
VkFence pompeii::vulkan::VulkanFrameContext::GetInFlightFence()					const { return m_InFlightFences; }
