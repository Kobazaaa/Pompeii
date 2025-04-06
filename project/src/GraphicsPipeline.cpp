#include "GraphicsPipeline.h"
#include <stdexcept>

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  GraphicsPipelineLayout	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::GraphicsPipelineLayout::Destroy(const Device& device)		const { vkDestroyPipelineLayout(device.GetDevice(), m_Layout, nullptr); }

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkPipelineLayout& pom::GraphicsPipelineLayout::GetLayout()	const { return m_Layout; }



//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  GraphicsPipelineLayoutBuilder	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::GraphicsPipelineLayoutBuilder::GraphicsPipelineLayoutBuilder()
{
	m_PipelineLayoutInfo = {};

	m_PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;		// CAN'T CHANGE
	m_PipelineLayoutInfo.setLayoutCount = 0;										//! REQUIRED CHANGE
	m_PipelineLayoutInfo.pSetLayouts = nullptr;										//! REQUIRED CHANGE
	m_PipelineLayoutInfo.pushConstantRangeCount = 0;								//? CAN CHANGE
	m_PipelineLayoutInfo.pPushConstantRanges = nullptr;								//? CAN CHANGE
}


//--------------------------------------------------
//    Builder
//--------------------------------------------------
pom::GraphicsPipelineLayoutBuilder& pom::GraphicsPipelineLayoutBuilder::NewPushConstantRange()
{
	m_vPushConstantRanges.emplace_back();
	m_vPushConstantRanges.back().offset = 0;
	return *this;
}
pom::GraphicsPipelineLayoutBuilder& pom::GraphicsPipelineLayoutBuilder::SetPCStageFlags(VkPipelineStageFlags flags) { m_vPushConstantRanges.back().stageFlags = flags; return *this; }
pom::GraphicsPipelineLayoutBuilder& pom::GraphicsPipelineLayoutBuilder::SetPCOffset(uint32_t offset) { m_vPushConstantRanges.back().offset = offset; return *this; }
pom::GraphicsPipelineLayoutBuilder& pom::GraphicsPipelineLayoutBuilder::SetPCSize(uint32_t size) { m_vPushConstantRanges.back().size = size; return *this; }

pom::GraphicsPipelineLayoutBuilder& pom::GraphicsPipelineLayoutBuilder::AddLayout(const pom::DescriptorSetLayout& descriptorSetLayout)
{
	m_vDescriptorLayouts.push_back(descriptorSetLayout.GetLayout());
	m_PipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_vDescriptorLayouts.size());
	m_PipelineLayoutInfo.pSetLayouts = m_vDescriptorLayouts.data();
	return *this;
}

void pom::GraphicsPipelineLayoutBuilder::Build(const Device& device, GraphicsPipelineLayout& pipelineLayout)
{
	if (!m_vPushConstantRanges.empty())
	{
		m_PipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(m_vPushConstantRanges.size());
		m_PipelineLayoutInfo.pPushConstantRanges = m_vPushConstantRanges.data();
	}

	if (vkCreatePipelineLayout(device.GetDevice(), &m_PipelineLayoutInfo, nullptr, &pipelineLayout.m_Layout) != VK_SUCCESS)
		throw std::runtime_error("failed to create Pipeline Layout!");
}
