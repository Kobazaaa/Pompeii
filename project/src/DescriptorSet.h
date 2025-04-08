#ifndef DESCRIPTOR_SET_H
#define DESCRIPTOR_SET_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <vector>

// -- Forward Declarations --
namespace pom
{
	class Buffer;
	class Image;
	class Sampler;
	struct Context;
}


namespace pom
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
		DescriptorSetLayout() = default;
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
		DescriptorSetLayoutBuilder& NewLayoutBinding();
		//! REQUIRED
		DescriptorSetLayoutBuilder& SetType(VkDescriptorType type);
		// If not called, assumes 1 by default
		DescriptorSetLayoutBuilder& SetCount(uint32_t count);
		//! REQUIRED
		DescriptorSetLayoutBuilder& SetShaderStages(VkShaderStageFlags flags);

		void Build(const Context& context, DescriptorSetLayout& descriptorSetLayout);

	private:
		std::vector<VkDescriptorSetLayoutBinding> m_vLayoutBindings;
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
		DescriptorSet() = default;

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
		DescriptorSetWriter& WriteBuffers(const DescriptorSet& set, uint32_t binding);

		DescriptorSetWriter& AddImageInfo(const Image& image, const Sampler& sampler);
		DescriptorSetWriter& WriteImages(const DescriptorSet& set, uint32_t binding);

		void Execute(const Context& context);

	private:
		std::vector<VkWriteDescriptorSet> m_vDescriptorWrites;
		std::vector<VkDescriptorBufferInfo> m_vBufferInfos;
		std::vector<VkDescriptorImageInfo> m_vImageInfos;
	};
}

#endif // DESCRIPTOR_SET_H