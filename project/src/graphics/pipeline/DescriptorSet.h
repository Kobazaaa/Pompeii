#ifndef DESCRIPTOR_SET_H
#define DESCRIPTOR_SET_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <vector>

// -- Forward Declarations --
namespace pompeii
{
	class ImageView;
	class Buffer;
	class Sampler;
	struct Context;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  DescriptorSetLayout	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class DescriptorSetLayout final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit DescriptorSetLayout() = default;
		void Destroy(const Context& context) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkDescriptorSetLayout& GetHandle() const;
		const std::vector<VkDescriptorSetLayoutBinding>& GetBindings() const;

	private:
		VkDescriptorSetLayout m_Layout;
		std::vector<VkDescriptorSetLayoutBinding> m_vLayoutBindings;
		friend class DescriptorSetLayoutBuilder;
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  DescriptorSetLayoutBuilder	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class DescriptorSetLayoutBuilder final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		DescriptorSetLayoutBuilder() = default;

		//--------------------------------------------------
		//    Builder
		//--------------------------------------------------
		DescriptorSetLayoutBuilder& SetDebugName(const char* name);
		DescriptorSetLayoutBuilder& NewLayoutBinding();
		//! REQUIRED
		DescriptorSetLayoutBuilder& SetType(VkDescriptorType type);
		// If not called, assumes 1 by default
		DescriptorSetLayoutBuilder& SetCount(uint32_t count);
		//! REQUIRED
		DescriptorSetLayoutBuilder& SetShaderStages(VkShaderStageFlags flags);
		DescriptorSetLayoutBuilder& AddLayoutFlag(VkDescriptorSetLayoutCreateFlags flags);
		DescriptorSetLayoutBuilder& AddBindingFlags(VkDescriptorBindingFlags flags);

		void Build(const Context& context, DescriptorSetLayout& descriptorSetLayout);

	private:
		const char* m_pName{ };
		std::vector<VkDescriptorSetLayoutBinding> m_vLayoutBindings;
		std::vector<VkDescriptorBindingFlags> m_vBindingFlags;
		VkDescriptorSetLayoutCreateFlags m_LayoutFlags{};
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  DescriptorSet
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class DescriptorSet final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit DescriptorSet() = default;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkDescriptorSet& GetHandle() const;
		const DescriptorSetLayout& GetLayout() const;

	private:
		VkDescriptorSet m_DescriptorSet;
		DescriptorSetLayout m_Layout;

		friend class DescriptorPool;
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  DescriptorSetWriter	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class DescriptorSetWriter final
	{
	public:
		DescriptorSetWriter() = default;

		//--------------------------------------------------
		//    Writing
		//--------------------------------------------------
		DescriptorSetWriter& AddBufferInfo(const Buffer& buffer, uint32_t offset, uint32_t range);
		DescriptorSetWriter& WriteBuffers(const DescriptorSet& set, uint32_t binding, uint32_t count = 0xFFFFFFFF);

		DescriptorSetWriter& AddImageInfo(const ImageView& view, VkImageLayout layout, const Sampler& sampler);
		DescriptorSetWriter& AddImageInfo(const ImageView& view, VkImageLayout layout);
		DescriptorSetWriter& WriteImages(const DescriptorSet& set, uint32_t binding, uint32_t count = 0xFFFFFFFF, uint32_t arraySlot = 0);

		void Execute(const Context& context);

	private:
		std::vector<VkWriteDescriptorSet> m_vDescriptorWrites;
		std::vector<VkDescriptorBufferInfo> m_vBufferInfos;
		std::vector<VkDescriptorImageInfo> m_vImageInfos;
	};
}

#endif // DESCRIPTOR_SET_H