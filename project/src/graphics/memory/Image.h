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
		explicit Image() = default;
		~Image() = default;
		Image(const Image& other) = delete;
		Image(Image&& other) noexcept;
		Image& operator=(const Image& other) = delete;
		Image& operator=(Image&& other) noexcept;
		void Destroy(const Context& context) const;

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		VkImageView& CreateView(const Context& context, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType viewType,
								uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount);
		static VkFormat FindSupportedFormat(const PhysicalDevice& physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkImage&		GetHandle()				const;
		const VkImageView&	GetViewHandle()			const;

		uint32_t			GetMipLevels()			const;
		uint32_t			GetLayerCount()			const;

		VkFormat			GetFormat()				const;
		VkExtent3D			GetExtent3D()			const;
		VkExtent2D			GetExtent2D()			const;
		VkImageLayout		GetCurrentLayout()		const;

		bool				HasStencilComponent()	const;
		bool				HasDepthComponent()		const;

		//--------------------------------------------------
		//    Commands
		//--------------------------------------------------
		void TransitionLayout(const CommandBuffer& cmd, VkImageLayout newLayout,
							  VkAccessFlags2 srcAccess, VkPipelineStageFlags2 srcStage,
							  VkAccessFlags2 dstAccess, VkPipelineStageFlags2 dstStage,
							  uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount);
		void InsertBarrier(const CommandBuffer& cmd,
							  VkAccessFlags2 srcAccess, VkPipelineStageFlags2 srcStage,
							  VkAccessFlags2 dstAccess, VkPipelineStageFlags2 dstStage);
		void GenerateMipMaps(const Context& context, const CommandBuffer& cmd, uint32_t texW, uint32_t texH, uint32_t mips, VkImageLayout finalLayout);

	private:
		VkImage m_Image					{ VK_NULL_HANDLE };
		VkImageView m_ImageView			{ VK_NULL_HANDLE };
		VmaAllocation m_ImageMemory		{ VK_NULL_HANDLE };

		VkImageLayout m_CurrentLayout	{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkImageCreateInfo m_ImageInfo	{ };

		friend class ImageBuilder;
		friend class SwapChainBuilder;
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
		ImageBuilder& SetPreMadeImage(VkImage image);

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

		VkImage m_PreMadeImage;
		VkImageCreateInfo m_ImageInfo{};
		VmaAllocationCreateInfo m_AllocInfo{};
	};
}

#endif // IMAGE_H