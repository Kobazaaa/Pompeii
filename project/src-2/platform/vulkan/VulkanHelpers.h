#ifndef POMPEII_VULKAN_HELPERS_H
#define POMPEII_VULKAN_HELPERS_H

// -- Standard Library --
#include <stdexcept>

// -- Vulkan Includes --
#include "vulkan/vulkan.h"

// -- Pompeii Includes --
#include "IImage.h"
#include "IBuffer.h"
#include "ICommandBuffer.h"

namespace pompeii
{
	static VkFormat TO_VK_FORMAT(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::RGBA8:			return VK_FORMAT_R8G8B8A8_SRGB;
		case ImageFormat::RGBA16F:			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case ImageFormat::RGBA32F:			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case ImageFormat::Depth24Stencil8:	return VK_FORMAT_D24_UNORM_S8_UINT;
		case ImageFormat::Depth32F:			return VK_FORMAT_D32_SFLOAT;
		}

		throw(std::out_of_range("Format out of range, not supported!"));
	}

	static VkImageUsageFlags TO_VK_USAGE(ImageUsage usage)
	{
		VkImageUsageFlags flags = 0;
		if (HasAnyFlags(usage, ImageUsage::ColorAttachment))		flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (HasAnyFlags(usage, ImageUsage::DepthStencilAttachment)) flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		if (HasAnyFlags(usage, ImageUsage::Sampled))				flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
		if (HasAnyFlags(usage, ImageUsage::Storage))				flags |= VK_IMAGE_USAGE_STORAGE_BIT;

		return flags;
	}
	static VkBufferUsageFlags TO_VK_USAGE(BufferUsage usage)
	{
		VkBufferUsageFlags flags = 0;
		if (HasAnyFlags(usage, BufferUsage::Vertex))		flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		if (HasAnyFlags(usage, BufferUsage::Index))			flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (HasAnyFlags(usage, BufferUsage::Uniform))		flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (HasAnyFlags(usage, BufferUsage::Storage))		flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		if (HasAnyFlags(usage, BufferUsage::Staging))		flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		return flags;
	}

	static VkMemoryPropertyFlags TO_VK_MEMORY(MemoryType memory)
	{
		switch (memory)
		{
		case MemoryType::DeviceLocal:	return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		case MemoryType::HostVisible:	return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}

		throw(std::out_of_range("Memory type out of range, not supported!"));
	}

	static VkCommandBufferLevel TO_VK_LEVEL(CommandBufferLevel level)
	{
		switch (level)
		{
		case CommandBufferLevel::Primary:	return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		case CommandBufferLevel::Secondary:	return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		}

		throw(std::out_of_range("Command buffer level out of range, not supported!"));
	}
}

#endif // POMPEII_VULKAN_HELPERS_H
