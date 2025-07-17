#ifndef CONTEXT_H
#define CONTEXT_H

// -- Vulkan Includes --
#include "vma/vk_mem_alloc.h"

// -- Pompeii Includes --
#include "Instance.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "DeletionQueue.h"
#include "CommandPool.h"
#include "DescriptorPool.h"

namespace pompeii
{
	struct Context
	{
		Instance		instance		{};
		VmaAllocator	allocator		{};
		PhysicalDevice	physicalDevice	{};
		Device			device			{};

		CommandPool*	commandPool		{};
		DescriptorPool*	descriptorPool	{};

		DeletionQueue	deletionQueue	{};
	};
}

#endif // CONTEXT_H