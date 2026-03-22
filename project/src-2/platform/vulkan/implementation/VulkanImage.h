#ifndef POMPEII_VULKAN_IMAGE_IMPLEMENTATION_H
#define POMPEII_VULKAN_IMAGE_IMPLEMENTATION_H

// -- Pompeii Includes --
#include "IImage.h"

// -- Vulkan Includes --
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

// -- Forward Declaration --
namespace pompeii::vulkan
{
	struct VulkanContext;
}

namespace pompeii::vulkan
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    VulkanImage
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class VulkanImage final : public IImage
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit VulkanImage(const ImageDesc& imageDesc, VulkanContext& context);
		explicit VulkanImage(VulkanContext& context, VkImage image, VkImageView view, VkFormat format);
		~VulkanImage() override;

		VulkanImage(const VulkanImage& other) = default;
		VulkanImage(VulkanImage&& other) noexcept = default;
		VulkanImage& operator=(const VulkanImage& other) = default;
		VulkanImage& operator=(VulkanImage&& other) noexcept = default;

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		void SetData(const void* data, size_t size, uint32_t mipLevel, uint32_t arrayLayer) override;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		[[nodiscard]] uint32_t GetWidth() const override;
		[[nodiscard]] uint32_t GetHeight() const override;
		[[nodiscard]] ImageFormat GetFormat() const override;
		[[nodiscard]] ImageUsage GetUsage() const override;
		[[nodiscard]] MemoryType GetMemoryType() const override;

		[[nodiscard]] VkImage GetHandle() const;

		//--------------------------------------------------
		//    Debug
		//--------------------------------------------------
		void SetDebugName(const char* name) override;

	private:
		// Context
		VulkanContext* m_pContext;

		// Image Info
		ImageDesc m_ImageDesc;
		VkFormat m_Format				{ VK_FORMAT_UNDEFINED };
		VkExtent3D m_Extent				{ 0, 0, 0 };
		VkImageLayout m_CurrentLayout	{ VK_IMAGE_LAYOUT_UNDEFINED };

		// Vulkan Data
		VkImage m_Image					{ VK_NULL_HANDLE };
		VmaAllocation m_Memory			{ nullptr };
	};
}

#endif // POMPEII_VULKAN_IMAGE_IMPLEMENTATION_H
