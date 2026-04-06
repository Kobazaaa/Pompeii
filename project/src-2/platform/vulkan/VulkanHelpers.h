#ifndef POMPEII_VULKAN_HELPERS_H
#define POMPEII_VULKAN_HELPERS_H

// -- Standard Library --
#include <stdexcept>

// -- Vulkan Includes --
#include "vulkan/vulkan.h"

// -- Pompeii Includes --
#include "PompeiiEnums.h"

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Image
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
	static ImageFormat TO_POMPEII_FORMAT (VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_R8G8B8A8_SRGB:			return ImageFormat::RGBA8;
		case VK_FORMAT_B8G8R8A8_SRGB:			return ImageFormat::RGBA8;
		case VK_FORMAT_R16G16B16A16_SFLOAT:		return ImageFormat::RGBA16F;
		case VK_FORMAT_R32G32B32A32_SFLOAT:		return ImageFormat::RGBA32F;
		case VK_FORMAT_D24_UNORM_S8_UINT:		return ImageFormat::Depth24Stencil8;
		case VK_FORMAT_D32_SFLOAT:				return ImageFormat::Depth32F;
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

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Buffer
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Memory
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	static VkMemoryPropertyFlags TO_VK_MEMORY(MemoryType memory)
	{
		switch (memory)
		{
		case MemoryType::DeviceLocal:	return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		case MemoryType::HostVisible:	return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}

		throw(std::out_of_range("Memory type out of range, not supported!"));
	}


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Command Buffer
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	static VkCommandBufferLevel TO_VK_LEVEL(CommandBufferLevel level)
	{
		switch (level)
		{
		case CommandBufferLevel::Primary:	return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		case CommandBufferLevel::Secondary:	return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		}

		throw(std::out_of_range("Command buffer level out of range, not supported!"));
	}


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Pipeline
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	static VkCullModeFlags TO_VK_CULL_MODE(CullMode mode)
	{
		switch (mode)
		{
		case CullMode::None:	return VK_CULL_MODE_NONE;
		case CullMode::Front:	return VK_CULL_MODE_FRONT_BIT;
		case CullMode::Back:	return VK_CULL_MODE_BACK_BIT;
		case CullMode::Both:	return VK_CULL_MODE_FRONT_AND_BACK;
		}

		throw(std::out_of_range("Cull mode out of range, not supported!"));
	}
	static VkFrontFace TO_VK_FRONT_FACE(FrontFace frontFace)
	{
		switch (frontFace)
		{
		case FrontFace::Clockwise:			return VK_FRONT_FACE_CLOCKWISE;
		case FrontFace::CounterClockwise:	return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		}

		throw(std::out_of_range("Front face out of range, not supported!"));
	}
	static VkPrimitiveTopology TO_VK_PRIMITIVE_TOPOLOGY(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case PrimitiveTopology::TriangleList:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}

		throw(std::out_of_range("Primitive topology out of range, not supported!"));
	}
	static VkPolygonMode TO_VK_POLYGON_MODE(PolygonMode polyMode)
	{
		switch (polyMode)
		{
		case PolygonMode::Fill:	return VK_POLYGON_MODE_FILL;
		}

		throw(std::out_of_range("Polygon mode out of range, not supported!"));
	}
	static VkCompareOp TO_VK_DEPTH_COMPARE_OP(DepthCompareOp op)
	{
		switch (op)
		{
		case DepthCompareOp::Never:				return VK_COMPARE_OP_NEVER;
		case DepthCompareOp::Less:				return VK_COMPARE_OP_LESS;
		case DepthCompareOp::Equal:				return VK_COMPARE_OP_EQUAL;
		case DepthCompareOp::LessOrEqual:		return VK_COMPARE_OP_LESS_OR_EQUAL;
		case DepthCompareOp::Greater:			return VK_COMPARE_OP_GREATER;
		case DepthCompareOp::NotEqual:			return VK_COMPARE_OP_NOT_EQUAL;
		case DepthCompareOp::GreaterOrEqual:	return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case DepthCompareOp::Always:			return VK_COMPARE_OP_ALWAYS;
		}

		throw(std::out_of_range("Depth compare operation out of range, not supported!"));
	}

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Shader
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	static VkShaderStageFlagBits TO_VK_SHADER_STAGE(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex:	return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderStage::Fragment:	return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderStage::Geometry:	return VK_SHADER_STAGE_GEOMETRY_BIT;
		case ShaderStage::Compute:	return VK_SHADER_STAGE_COMPUTE_BIT;
		}

		throw(std::out_of_range("Shader stage out of range, not supported!"));
	}

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Vertex
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	static VkVertexInputRate TO_VK_VERTEX_INPUT_RATE(VertexInputRate rate)
	{
		switch (rate)
		{
		case VertexInputRate::Vertex:	return VK_VERTEX_INPUT_RATE_VERTEX;
		case VertexInputRate::Instance:	return VK_VERTEX_INPUT_RATE_INSTANCE;
		}

		throw(std::out_of_range("Vertex input rate out of range, not supported!"));
	}
	static VkFormat TO_VK_VERTEX_ATTRIBUTE_FORMAT(VertexFormat format)
	{
		switch (format)
		{
		case VertexFormat::Float:	return VK_FORMAT_R32_SFLOAT;
		case VertexFormat::Float2:	return VK_FORMAT_R32G32_SFLOAT;
		case VertexFormat::Float3:	return VK_FORMAT_R32G32B32_SFLOAT;
		case VertexFormat::Float4:	return VK_FORMAT_R32G32B32A32_SFLOAT;
		}

		throw(std::out_of_range("Vertex attribute format out of range, not supported!"));
	}

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Load/Store Ops
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	static VkAttachmentLoadOp TO_VK_LOAD_OP(LoadOp op)
	{
		switch (op)
		{
		case LoadOp::Load:		return VK_ATTACHMENT_LOAD_OP_LOAD;
		case LoadOp::Clear:		return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case LoadOp::DontCare:	return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}

		throw(std::out_of_range("Load operation out of range, not supported!"));
	}

	static VkAttachmentStoreOp TO_VK_STORE_OP(StoreOp op)
	{
		switch (op)
		{
		case StoreOp::Store:	return VK_ATTACHMENT_STORE_OP_STORE;
		case StoreOp::DontCare:	return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}

		throw(std::out_of_range("Store operation out of range, not supported!"));
	}
}

#endif // POMPEII_VULKAN_HELPERS_H
