#include "DescriptorSet.h"

#include <stdexcept>


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  DescriptorSetLayout	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::DescriptorSetLayout::Destroy(const Device& device) const { vkDestroyDescriptorSetLayout(device.GetDevice(), m_Layout, nullptr); }

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkDescriptorSetLayout& pom::DescriptorSetLayout::GetLayout() const { return m_Layout; }
const std::vector<VkDescriptorSetLayoutBinding>& pom::DescriptorSetLayout::GetBindings() const { return m_vLayoutBindings; }


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  DescriptorSetLayoutBuilder	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Builder
//--------------------------------------------------
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

void pom::DescriptorSetLayoutBuilder::Build(const Device& device, DescriptorSetLayout& descriptorSetLayout) const
{
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(m_vLayoutBindings.size());
	layoutInfo.pBindings = m_vLayoutBindings.data();

	descriptorSetLayout.m_vLayoutBindings = m_vLayoutBindings;

	if (vkCreateDescriptorSetLayout(device.GetDevice(), &layoutInfo, nullptr, &descriptorSetLayout.m_Layout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Descriptor Set Layout!");
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

pom::DescriptorSetWriter& pom::DescriptorSetWriter::WriteBuffer(const DescriptorSet& set, uint32_t binding, const Buffer& buffer, uint32_t offset, uint32_t range)
{
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer.GetBuffer();
	bufferInfo.offset = offset;
	bufferInfo.range = range;
	m_vBufferInfos.push_back(bufferInfo);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = set.GetHandle();
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorType = set.GetLayout().GetBindings()[binding].descriptorType;
	write.descriptorCount = set.GetLayout().GetBindings()[binding].descriptorCount;
	write.pBufferInfo = &m_vBufferInfos.back();
	write.pImageInfo = nullptr;
	write.pTexelBufferView = nullptr;
	m_vDescriptorWrites.push_back(write);

	return *this;
}

pom::DescriptorSetWriter& pom::DescriptorSetWriter::WriteImage(const DescriptorSet& set, uint32_t binding, const Image& image, const Sampler& sampler)
{
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = image.GetCurrentLayout();
	imageInfo.imageView = image.GetImageView();
	imageInfo.sampler = sampler.GetSampler();
	m_vImageInfos.push_back(imageInfo);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = set.GetHandle();
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorType = set.GetLayout().GetBindings()[binding].descriptorType;
	write.descriptorCount = set.GetLayout().GetBindings()[binding].descriptorCount;
	write.pImageInfo = &m_vImageInfos.back();
	write.pBufferInfo = nullptr;
	write.pTexelBufferView = nullptr;
	m_vDescriptorWrites.push_back(write);

	return *this;
}

void pom::DescriptorSetWriter::Execute(const Device& device)
{
	vkUpdateDescriptorSets(device.GetDevice(), static_cast<uint32_t>(m_vDescriptorWrites.size()), m_vDescriptorWrites.data(), 0, nullptr);

	m_vDescriptorWrites.clear();
	m_vImageInfos.clear();
	m_vBufferInfos.clear();
}
