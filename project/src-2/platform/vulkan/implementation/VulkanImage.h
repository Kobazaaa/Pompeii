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
		explicit VulkanImage(VulkanContext& context, VkImage image, VkImageView view, VkFormat format, VkExtent2D extent);
		~VulkanImage() override;

		VulkanImage(const VulkanImage& other) = delete;
		VulkanImage(VulkanImage&& other) noexcept = default;
		VulkanImage& operator=(const VulkanImage& other) = delete;
		VulkanImage& operator=(VulkanImage&& other) noexcept = default;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		[[nodiscard]] uint32_t GetWidth() const override;
		[[nodiscard]] uint32_t GetHeight() const override;
		[[nodiscard]] ImageFormat GetFormat() const override;
		[[nodiscard]] ImageUsage GetUsage() const override;
		[[nodiscard]] MemoryType GetMemoryType() const override;

		//--------------------------------------------------
		//    Operator Overloads
		//--------------------------------------------------
		operator VkImage() const;
		operator VkImageView() const;
		operator VmaAllocation() const;

	private:
		void CreateImageView();

		// Context
		VulkanContext* m_pContext;

		// Image Info
		ImageDesc m_ImageDesc;
		VkImageLayout m_CurrentLayout	{ VK_IMAGE_LAYOUT_UNDEFINED };

		// Vulkan Data
		VkImage m_Image					{ VK_NULL_HANDLE };
		VkImageView m_View				{ VK_NULL_HANDLE };
		VmaAllocation m_Memory			{ nullptr };

		// todo let's not
		friend class VulkanCommandBuffer;
	};
}

#endif // POMPEII_VULKAN_IMAGE_IMPLEMENTATION_H
