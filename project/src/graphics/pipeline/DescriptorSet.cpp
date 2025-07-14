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
void pompeii::DescriptorSetLayout::Destroy(const Context& context) const { vkDestroyDescriptorSetLayout(context.device.GetHandle(), m_Layout, nullptr); }

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkDescriptorSetLayout& pompeii::DescriptorSetLayout::GetHandle() const { return m_Layout; }
const std::vector<VkDescriptorSetLayoutBinding>& pompeii::DescriptorSetLayout::GetBindings() const { return m_vLayoutBindings; }


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  DescriptorSetLayoutBuilder	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Builder
//--------------------------------------------------
pompeii::DescriptorSetLayoutBuilder& pompeii::DescriptorSetLayoutBuilder::SetDebugName(const char* name)
{
	m_pName = name;
	return *this;
}
pompeii::DescriptorSetLayoutBuilder& pompeii::DescriptorSetLayoutBuilder::NewLayoutBinding()
{
	m_vLayoutBindings.emplace_back();
	m_vLayoutBindings.back().binding = static_cast<uint32_t>(m_vLayoutBindings.size() - 1);
	m_vLayoutBindings.back().descriptorCount = 1;
	m_vLayoutBindings.back().pImmutableSamplers = nullptr;

	m_vBindingFlags.emplace_back();
	return *this;
}
pompeii::DescriptorSetLayoutBuilder& pompeii::DescriptorSetLayoutBuilder::SetType(VkDescriptorType type)						{ m_vLayoutBindings.back().descriptorType = type; return *this; }
pompeii::DescriptorSetLayoutBuilder& pompeii::DescriptorSetLayoutBuilder::SetCount(uint32_t count)								{ m_vLayoutBindings.back().descriptorCount = count; return *this; }
pompeii::DescriptorSetLayoutBuilder& pompeii::DescriptorSetLayoutBuilder::SetShaderStages(VkShaderStageFlags flags)				{ m_vLayoutBindings.back().stageFlags = flags; return *this; }
pompeii::DescriptorSetLayoutBuilder& pompeii::DescriptorSetLayoutBuilder::AddLayoutFlag(VkDescriptorSetLayoutCreateFlags flags) { m_LayoutFlags |= flags; return *this; }
pompeii::DescriptorSetLayoutBuilder& pompeii::DescriptorSetLayoutBuilder::AddBindingFlags(VkDescriptorBindingFlags flags)		{ m_vBindingFlags.back() |= flags; return *this; }
void pompeii::DescriptorSetLayoutBuilder::Build(const Context& context, DescriptorSetLayout& descriptorSetLayout)
{
	VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
	flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	flagsInfo.bindingCount = static_cast<uint32_t>(m_vBindingFlags.size());
	flagsInfo.pBindingFlags = m_vBindingFlags.data();

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(m_vLayoutBindings.size());
	layoutInfo.pBindings = m_vLayoutBindings.data();
	layoutInfo.flags = m_LayoutFlags;
	layoutInfo.pNext = &flagsInfo;

	descriptorSetLayout.m_vLayoutBindings = m_vLayoutBindings;

	if (vkCreateDescriptorSetLayout(context.device.GetHandle(), &layoutInfo, nullptr, &descriptorSetLayout.m_Layout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Descriptor Set Layout!");

	if (m_pName)
	{
		Debugger::SetDebugObjectName(reinterpret_cast<uint64_t>(descriptorSetLayout.GetHandle()), VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, m_pName);
	}

	m_pName = nullptr;
	m_vLayoutBindings.clear();
	m_vBindingFlags.clear();
	m_LayoutFlags = {};
}



//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  DescriptorSet
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkDescriptorSet& pompeii::DescriptorSet::GetHandle() const { return m_DescriptorSet; }
const pompeii::DescriptorSetLayout& pompeii::DescriptorSet::GetLayout() const { return m_Layout; }




//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  DescriptorSetWriter	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Writing
//--------------------------------------------------

pompeii::DescriptorSetWriter& pompeii::DescriptorSetWriter::AddBufferInfo(const Buffer& buffer, uint32_t offset, uint32_t range)
{
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer.GetHandle();
	bufferInfo.offset = offset;
	bufferInfo.range = range;
	m_vBufferInfos.push_back(bufferInfo);

	return *this;
}
pompeii::DescriptorSetWriter& pompeii::DescriptorSetWriter::WriteBuffers(const DescriptorSet& set, uint32_t binding, uint32_t count)
{
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = set.GetHandle();
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorType = set.GetLayout().GetBindings()[binding].descriptorType;
	write.descriptorCount = count == 0xFFFFFFFF ? set.GetLayout().GetBindings()[binding].descriptorCount : count;
	write.pBufferInfo = m_vBufferInfos.data();
	write.pImageInfo = nullptr;
	write.pTexelBufferView = nullptr;
	m_vDescriptorWrites.push_back(write);

	return *this;
}

pompeii::DescriptorSetWriter& pompeii::DescriptorSetWriter::AddImageInfo(const ImageView& view, VkImageLayout layout, const Sampler& sampler)
{
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = layout;
	imageInfo.imageView = view.GetHandle();
	imageInfo.sampler = sampler.GetHandle();
	m_vImageInfos.push_back(imageInfo);

	return *this;
}
pompeii::DescriptorSetWriter& pompeii::DescriptorSetWriter::AddImageInfo(const ImageView& view, VkImageLayout layout)
{
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = layout;
	imageInfo.imageView = view.GetHandle();
	imageInfo.sampler = VK_NULL_HANDLE;
	m_vImageInfos.push_back(imageInfo);

	return *this;
}
pompeii::DescriptorSetWriter& pompeii::DescriptorSetWriter::WriteImages(const DescriptorSet& set, uint32_t binding, uint32_t count, uint32_t arraySlot)
{
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = set.GetHandle();
	write.dstBinding = binding;
	write.dstArrayElement = arraySlot;
	write.descriptorType = set.GetLayout().GetBindings()[binding].descriptorType;
	write.descriptorCount = count == 0xFFFFFFFF ? set.GetLayout().GetBindings()[binding].descriptorCount : count;
	write.pImageInfo = m_vImageInfos.data();
	write.pBufferInfo = nullptr;
	write.pTexelBufferView = nullptr;
	m_vDescriptorWrites.push_back(write);

	return *this;
}

void pompeii::DescriptorSetWriter::Execute(const Context& context)
{
	vkUpdateDescriptorSets(context.device.GetHandle(), static_cast<uint32_t>(m_vDescriptorWrites.size()), m_vDescriptorWrites.data(), 0, nullptr);

	m_vDescriptorWrites.clear();
	m_vImageInfos.clear();
	m_vBufferInfos.clear();
}
