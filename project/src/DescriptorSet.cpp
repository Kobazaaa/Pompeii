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

	if (vkCreateDescriptorSetLayout(device.GetDevice(), &layoutInfo, nullptr, &descriptorSetLayout.m_Layout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Descriptor Set Layout!");
}
