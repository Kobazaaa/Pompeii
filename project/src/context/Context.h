#ifndef CONTEXT_H
#define CONTEXT_H

// -- Vulkan Includes --
#include "vma/vk_mem_alloc.h"

// -- Pompeii Includes --
#include "Instance.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "DeletionQueue.h"
#include "DescriptorPool.h"
#include "CommandPool.h"

namespace pom
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