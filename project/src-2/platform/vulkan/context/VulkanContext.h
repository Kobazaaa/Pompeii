#ifndef POMPEII_VULKAN_CONTEXT_H
#define POMPEII_VULKAN_CONTEXT_H

// -- Vulkan Includes --
#include <vma/vk_mem_alloc.h>
#include "VulkanInstance.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanPhysicalDevice.h"

namespace pompeii::vulkan
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    VulkanContext
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct VulkanContext
	{
		Instance instance{};
		VkSurfaceKHR surface{};
		PhysicalDevice physicalDevice{};
		Device device{};
		VmaAllocator allocator{};
		VulkanSwapChain* swapchain{};
	};
}

#endif // POMPEII_VULKAN_CONTEXT_H
