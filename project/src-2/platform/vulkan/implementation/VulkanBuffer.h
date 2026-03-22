#ifndef POMPEII_VULKAN_BUFFER_IMPLEMENTATION_H
#define POMPEII_VULKAN_BUFFER_IMPLEMENTATION_H

// -- Pompeii Includes --
#include "IBuffer.h"

// -- Vulkan Includes --
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

// -- Forward Declarations --
namespace pompeii::vulkan
{
	struct VulkanContext;
}

namespace pompeii::vulkan
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    VulkanBuffer
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class VulkanBuffer final : public IBuffer
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit VulkanBuffer(const BufferDesc& bufferDesc, VulkanContext& context);
		~VulkanBuffer() override;

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		void SetData(const void* data, size_t size, size_t offset) override;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		[[nodiscard]] size_t GetSize() const override;
		[[nodiscard]] BufferUsage GetUsage() const override;
		[[nodiscard]] MemoryType GetMemoryType() const override;

		[[nodiscard]] VkBuffer GetHandle() const;

		//--------------------------------------------------
		//    Debug
		//--------------------------------------------------
		void SetDebugName(const char* name) override;

	private:
		// Context
		VulkanContext* m_pContext;

		// Image Info
		BufferDesc m_BufferDesc;

		// Vulkan Data
		VkBuffer m_Buffer				{ VK_NULL_HANDLE };
		VmaAllocation m_Memory			{ nullptr };
	};
}

#endif // POMPEII_VULKAN_BUFFER_IMPLEMENTATION_H
