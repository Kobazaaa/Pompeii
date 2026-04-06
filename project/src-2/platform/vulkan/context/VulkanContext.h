#ifndef POMPEII_VULKAN_CONTEXT_H
#define POMPEII_VULKAN_CONTEXT_H

// -- Vulkan Includes --
#include <vma/vk_mem_alloc.h>

// -- Pompeii Includes --
#include "Instance.h"
#include "SwapChain.h"
#include "Device.h"
#include "PhysicalDevice.h"

// -- Standard Library --
#include <memory>

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
		std::unique_ptr<VulkanSwapChain> swapchain{};
	};
}

#endif // POMPEII_VULKAN_CONTEXT_H
