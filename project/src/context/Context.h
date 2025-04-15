#ifndef CONTEXT_H
#define CONTEXT_H

// -- Vulkan Includes --
#include "vma/vk_mem_alloc.h"

// -- Pompeii Includes --
#include "Instance.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "DeletionQueue.h"

namespace pom
{
	struct Context
	{
		Instance		instance		{};
		VmaAllocator	allocator		{};
		PhysicalDevice	physicalDevice	{};
		Device			device			{};

		DeletionQueue	deletionQueue	{};
	};
}

#endif // CONTEXT_H