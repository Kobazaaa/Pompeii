#ifndef IMAGE_H
#define IMAGE_H

// -- Vulkan Includes --
#include <vma/vk_mem_alloc.h>

// -- Standard Library --
#include <vector>
#include "CommandPool.h"

// -- Forward Declarations --
namespace pom
{
	class PhysicalDevice;
	class CommandPool;
	struct Context;
}


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
		explicit Image(VkImage image);
		void Destroy(const Context& context) const;

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		VkImageView& CreateView(const Context& context, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType viewType);
		static VkFormat FindSupportedFormat(const PhysicalDevice& physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkImage&		GetHandle()				const;
		const VkImageView&	GetViewHandle()			const;

		uint32_t			GetMipLevels()			const;
		uint32_t			GetLayerCount()			const;

		VkFormat			GetFormat()				const;
		VkImageLayout		GetCurrentLayout()		const;

		bool				HasStencilComponent()	const;

	private:
		VkImage m_Image					{ VK_NULL_HANDLE };
		VkImageView m_ImageView			{ VK_NULL_HANDLE };
		VmaAllocation m_ImageMemory		{ VK_NULL_HANDLE };

		VkImageLayout m_CurrentLayout	{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkImageCreateInfo m_ImageInfo	{ };

		friend class ImageBuilder;
		friend void pom::CommandPool::TransitionImageLayout(Image& image, VkImageLayout newLayout);
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
		ImageBuilder& SetDebugName(const char* name);						
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
		ImageBuilder& InitialData(void* data, uint32_t offset, uint32_t width, uint32_t height, uint32_t dataSize, VkImageLayout finalLayout, CommandPool& cmdPool);

		void Build(const Context& context, Image& image) const;

	private:
		bool m_UseInitialData;
		void* m_pData;
		uint32_t m_InitDataSize;
		uint32_t m_InitDataWidth;
		uint32_t m_InitDataHeight;
		uint32_t m_InitDataOffset;
		VkImageLayout m_FinalLayout;
		CommandPool* m_pCmdPool;

		const char* m_pName{};

		VkImageCreateInfo m_ImageInfo{};
		VmaAllocationCreateInfo m_AllocInfo{};
	};
}

#endif // IMAGE_H