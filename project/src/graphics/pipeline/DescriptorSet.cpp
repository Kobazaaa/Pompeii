// -- Standard Library --
#include <stdexcept>

// -- Pompeii Includes --
#include "DescriptorSet.h"
#include "Buffer.h"
#include "Context.h"
#include "Debugger.h"
#include "Image.h"
#include "Sampler.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  DescriptorSetLayout	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::DescriptorSetLayout::Destroy(const Context& context) const { vkDestroyDescriptorSetLayout(context.device.GetHandle(), m_Layout, nullptr); }

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkDescriptorSetLayout& pom::DescriptorSetLayout::GetHandle() const { return m_Layout; }
const std::vector<VkDescriptorSetLayoutBinding>& pom::DescriptorSetLayout::GetBindings() const { return m_vLayoutBindings; }


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  DescriptorSetLayoutBuilder	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Builder
//--------------------------------------------------
pom::DescriptorSetLayoutBuilder& pom::DescriptorSetLayoutBuilder::SetDebugName(const char* name)
{
	m_pName = name;
	return *this;
}
pom::DescriptorSetLayoutBuilder& pom::DescriptorSetLayoutBuilder::NewLayoutBinding()
{
	m_vLayoutBindings.emplace_back();
	m_vLayoutBindings.back().binding = static_cast<uint32_t>(m_vLayoutBindings.size() - 1);
	m_vLayoutBindings.back().descriptorCount = 1;
	m_vLayoutBindings.back().pImmutableSamplers = nullptr;
	return *this;
}
pom::DescriptorSetLayoutBuilder& pom::DescriptorSetLayoutBuilder::SetType(VkDescriptorType type)			{ m_vLayoutBindings.back().descriptorType = type; return *this; }
pom::DescriptorSetLayoutBuilder& pom::DescriptorSetLayoutBuilder::SetCount(uint32_t count)					{ m_vLayoutBindings.back().descriptorCount = count; return *this; }
pom::DescriptorSetLayoutBuilder& pom::DescriptorSetLayoutBuilder::SetShaderStages(VkShaderStageFlags flags) { m_vLayoutBindings.back().stageFlags = flags; return *this; }

void pom::DescriptorSetLayoutBuilder::Build(const Context& context, DescriptorSetLayout& descriptorSetLayout)
{
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(m_vLayoutBindings.size());
	layoutInfo.pBindings = m_vLayoutBindings.data();

	descriptorSetLayout.m_vLayoutBindings = m_vLayoutBindings;

	if (vkCreateDescriptorSetLayout(context.device.GetHandle(), &layoutInfo, nullptr, &descriptorSetLayout.m_Layout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Descriptor Set Layout!");

	if (m_pName)
	{
		Debugger::SetDebugObjectName(reinterpret_cast<uint64_t>(descriptorSetLayout.GetHandle()), VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, m_pName);
	}

	m_pName = nullptr;
	m_vLayoutBindings.clear();
}



//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  DescriptorSet
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkDescriptorSet& pom::DescriptorSet::GetHandle() const { return m_DescriptorSet; }
const pom::DescriptorSetLayout& pom::DescriptorSet::GetLayout() const {	return m_Layout; }




//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  DescriptorSetWriter	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Writing
//--------------------------------------------------

pom::DescriptorSetWriter& pom::DescriptorSetWriter::AddBufferInfo(const Buffer& buffer, uint32_t offset, uint32_t range)
{
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer.GetHandle();
	bufferInfo.offset = offset;
	bufferInfo.range = range;
	m_vBufferInfos.push_back(bufferInfo);

	return *this;
}
pom::DescriptorSetWriter& pom::DescriptorSetWriter::WriteBuffers(const DescriptorSet& set, uint32_t binding)
{
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = set.GetHandle();
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorType = set.GetLayout().GetBindings()[binding].descriptorType;
	write.descriptorCount = set.GetLayout().GetBindings()[binding].descriptorCount;
	write.pBufferInfo = m_vBufferInfos.data();
	write.pImageInfo = nullptr;
	write.pTexelBufferView = nullptr;
	m_vDescriptorWrites.push_back(write);

	return *this;
}

pom::DescriptorSetWriter& pom::DescriptorSetWriter::AddImageInfo(const Image& image, const Sampler& sampler)
{
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = image.GetCurrentLayout();
	imageInfo.imageView = image.GetViewHandle();
	imageInfo.sampler = sampler.GetHandle();
	m_vImageInfos.push_back(imageInfo);

	return *this;
}
pom::DescriptorSetWriter& pom::DescriptorSetWriter::WriteImages(const DescriptorSet& set, uint32_t binding)
{
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = set.GetHandle();
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorType = set.GetLayout().GetBindings()[binding].descriptorType;
	write.descriptorCount = set.GetLayout().GetBindings()[binding].descriptorCount;
	write.pImageInfo = m_vImageInfos.data();
	write.pBufferInfo = nullptr;
	write.pTexelBufferView = nullptr;
	m_vDescriptorWrites.push_back(write);

	return *this;
}

void pom::DescriptorSetWriter::Execute(const Context& context)
{
	vkUpdateDescriptorSets(context.device.GetHandle(), static_cast<uint32_t>(m_vDescriptorWrites.size()), m_vDescriptorWrites.data(), 0, nullptr);

	m_vDescriptorWrites.clear();
	m_vImageInfos.clear();
	m_vBufferInfos.clear();
}
