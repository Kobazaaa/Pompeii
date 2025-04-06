#ifndef DESCRIPTOR_SET_H
#define DESCRIPTOR_SET_H

#include <vector>

#include "Buffer.h"
#include "Device.h"
#include "Image.h"
#include "Sampler.h"

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
		void Destroy(const Device& device) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkDescriptorSetLayout& GetLayout() const;
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

		void Build(const Device& device, DescriptorSetLayout& descriptorSetLayout) const;

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
		DescriptorSetWriter& WriteBuffer(const DescriptorSet& set, uint32_t binding, const Buffer& buffer, uint32_t offset, uint32_t range);
		DescriptorSetWriter& WriteImage(const DescriptorSet& set, uint32_t binding, const Image& image, const Sampler& sampler);
		void Execute(const Device& device);

	private:
		std::vector<VkWriteDescriptorSet> m_vDescriptorWrites;
		std::vector<VkDescriptorBufferInfo> m_vBufferInfos;
		std::vector<VkDescriptorImageInfo> m_vImageInfos;
	};
}

#endif // DESCRIPTOR_SET_H