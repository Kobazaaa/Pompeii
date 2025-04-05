#ifndef IMAGE_H
#define IMAGE_H

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Device.h"

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Image	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Image final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		Image() = default;
		Image(VkImage image);
		void Destroy(Device& device, const VmaAllocator& allocator) const;

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void TransitionImageLayout(VkFormat format, VkImageLayout newLayout);
		VkImageView& GenerateImageView(Device& device, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType viewType);
		static VkFormat FindSupportedFormat(const PhysicalDevice& physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		VkImage& GetImage();
		VkImageView& GetImageView();
		VkFormat GetFormat() const;
		VkImageLayout GetCurrentLayout() const;

	private:
		VkImage m_Image					{ VK_NULL_HANDLE };
		VkImageView m_ImageView			{ VK_NULL_HANDLE };
		VmaAllocation m_ImageMemory		{ VK_NULL_HANDLE };

		VkImageLayout m_CurrentLayout	{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkImageCreateInfo m_ImageInfo	{};

		friend class ImageBuilder;
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  ImageBuilder	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class ImageBuilder final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		ImageBuilder();


		//--------------------------------------------------
		//    Builder
		//--------------------------------------------------
		//! REQUIRED
		ImageBuilder& SetWidth(uint32_t width);						
		//! REQUIRED
		ImageBuilder& SetHeight(uint32_t height);
		ImageBuilder& SetDepth(uint32_t depth);
		//! REQUIRED
		ImageBuilder& SetFormat(VkFormat format);
		ImageBuilder& SetTiling(VkImageTiling tiling);
		ImageBuilder& SetUsageFlags(VkImageUsageFlags usage);
		ImageBuilder& SetMemoryProperties(VkMemoryPropertyFlags properties);
		ImageBuilder& SetMipLevels(uint32_t levels);
		ImageBuilder& SetArrayLayers(uint32_t layers);
		ImageBuilder& SetSampleCount(VkSampleCountFlagBits sampleCount);
		ImageBuilder& SetSharingMode(VkSharingMode sharingMode);
		ImageBuilder& SetImageType(VkImageType type);

		ImageBuilder& Build(const VmaAllocator& allocator, Image& image);

	private:
		VkImageCreateInfo m_ImageInfo{};
		VmaAllocationCreateInfo m_AllocInfo{};
	};
}

#endif // IMAGE_H