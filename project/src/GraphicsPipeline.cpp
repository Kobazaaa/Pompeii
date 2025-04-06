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



//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  GraphicsPipeline
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::GraphicsPipeline::Destroy(const Device& device) const { vkDestroyPipeline(device.GetDevice(), m_Pipeline, nullptr); }

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkPipeline& pom::GraphicsPipeline::GetPipeline() const { return m_Pipeline; }



//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  GraphicsPipelineBuilder	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::GraphicsPipelineBuilder::GraphicsPipelineBuilder()
{
	m_VertexInputInfo = {};
	m_VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;		// CAN'T CHANGE
	m_VertexInputInfo.vertexBindingDescriptionCount = 0;										//! REQUIRED CHANGE
	m_VertexInputInfo.pVertexBindingDescriptions = nullptr;										//! REQUIRED CHANGE
	m_VertexInputInfo.vertexAttributeDescriptionCount = 0;										//! REQUIRED CHANGE
	m_VertexInputInfo.pVertexAttributeDescriptions = nullptr;									//! REQUIRED CHANGE

	m_InputAssembly = {};
	m_InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;		// CAN'T CHANGE
	m_InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;								//! REQUIRED CHANGE
	m_InputAssembly.primitiveRestartEnable = VK_FALSE;											// CAN'T CHANGE

	m_ViewportState = {};
	m_ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;				// CAN'T CHANGE
	m_ViewportState.viewportCount = 1;															// CAN'T CHANGE
	m_ViewportState.scissorCount = 1;															// CAN'T CHANGE

	m_RasterizerInfo = {};
	m_RasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;		// CAN'T CHANGE
	m_RasterizerInfo.depthClampEnable = VK_FALSE;												// CAN'T CHANGE
	m_RasterizerInfo.rasterizerDiscardEnable = VK_FALSE;										// CAN'T CHANGE
	m_RasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;										//! REQUIRED CHANGE
	m_RasterizerInfo.lineWidth = 1.0f;															// CAN'T CHANGE
	m_RasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;											//! REQUIRED CHANGE
	m_RasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;										//? CAN CHANGE
	m_RasterizerInfo.depthBiasEnable = VK_FALSE;												// CAN'T CHANGE

	m_MultiSamplingInfo = {};
	m_MultiSamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;		// CAN'T CHANGE
	m_MultiSamplingInfo.sampleShadingEnable = VK_FALSE;											// CAN'T CHANGE
	m_MultiSamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;							// CAN'T CHANGE

	m_DepthStencilInfo = {};
	m_DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;		// CAN'T CHANGE
	m_DepthStencilInfo.depthTestEnable = VK_TRUE;												//? CAN CHANGE
	m_DepthStencilInfo.depthWriteEnable = VK_TRUE;												//? CAN CHANGE
	m_DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;										//? CAN CHANGE
	m_DepthStencilInfo.depthBoundsTestEnable = VK_FALSE;										// CAN'T CHANGE
	m_DepthStencilInfo.minDepthBounds = 0.0f;													// CAN'T CHANGE
	m_DepthStencilInfo.maxDepthBounds = 1.0f;													// CAN'T CHANGE
	m_DepthStencilInfo.stencilTestEnable = VK_FALSE;											// CAN'T CHANGE
	m_DepthStencilInfo.front = {};																// CAN'T CHANGE
	m_DepthStencilInfo.back = {};																// CAN'T CHANGE

	m_ColorBlendAttachmentState = {};
	m_ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |						// CAN'T CHANGE
												 VK_COLOR_COMPONENT_G_BIT |
												 VK_COLOR_COMPONENT_B_BIT |
												 VK_COLOR_COMPONENT_A_BIT;
	m_ColorBlendAttachmentState.blendEnable = VK_FALSE;											// CAN'T CHANGE

	m_ColorBlendCreateInfo = {};
	m_ColorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;	// CAN'T CHANGE
	m_ColorBlendCreateInfo.logicOpEnable = VK_FALSE;											// CAN'T CHANGE							
	m_ColorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;											// CAN'T CHANGE
	m_ColorBlendCreateInfo.attachmentCount = 1;													// CAN'T CHANGE
	m_ColorBlendCreateInfo.pAttachments = &m_ColorBlendAttachmentState;							// CAN'T CHANGE
	m_ColorBlendCreateInfo.blendConstants[0] = 0.0f;											// CAN'T CHANGE
	m_ColorBlendCreateInfo.blendConstants[1] = 0.0f;											// CAN'T CHANGE
	m_ColorBlendCreateInfo.blendConstants[2] = 0.0f;											// CAN'T CHANGE
	m_ColorBlendCreateInfo.blendConstants[3] = 0.0f;											// CAN'T CHANGE

	m_DynamicStateInfo = {};
	m_DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;			// CAN'T CHANGE
	m_DynamicStateInfo.dynamicStateCount = 0;													//? CAN CHANGE
	m_DynamicStateInfo.pDynamicStates = nullptr;												//? CAN CHANGE

	m_PipelineLayout = VK_NULL_HANDLE;															//! REQUIRED CHANGE										
	m_RenderPass = VK_NULL_HANDLE;																//! REQUIRED CHANGE
}


//--------------------------------------------------
//    Builder
//--------------------------------------------------
// Shader Info
pom::GraphicsPipelineBuilder& pom::GraphicsPipelineBuilder::SetShader(const ShaderModule& shader, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderInfo{};
	shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderInfo.stage = stage;
	shaderInfo.module = shader.GetShader();
	shaderInfo.pName = "main";
	shaderInfo.pSpecializationInfo = nullptr;

	m_vShaderInfo.push_back(shaderInfo);

	return *this;
}

// Vertex Input Info
pom::GraphicsPipelineBuilder& pom::GraphicsPipelineBuilder::SetVertexBindingDesc(const VkVertexInputBindingDescription& desc)
{
	m_VertexInputInfo.vertexBindingDescriptionCount = 1;
	m_VertexInputInfo.pVertexBindingDescriptions = &desc;
	return *this;
}
pom::GraphicsPipelineBuilder& pom::GraphicsPipelineBuilder::SetVertexAttributeDesc(const std::vector<VkVertexInputAttributeDescription>& attr)
{
	m_VertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attr.size());
	m_VertexInputInfo.pVertexAttributeDescriptions = attr.data();
	return *this;
}

// Input Assembly Info
pom::GraphicsPipelineBuilder& pom::GraphicsPipelineBuilder::SetPrimitiveTopology(VkPrimitiveTopology topology)
{
	m_InputAssembly.topology = topology;
	return *this;
}

// Rasterizer Info
pom::GraphicsPipelineBuilder& pom::GraphicsPipelineBuilder::SetCullMode(VkCullModeFlags cullMode)
{
	m_RasterizerInfo.cullMode = cullMode;
	return *this;
}
pom::GraphicsPipelineBuilder& pom::GraphicsPipelineBuilder::SetPolygonMode(VkPolygonMode polyMode)
{
	m_RasterizerInfo.polygonMode = polyMode;
	return *this;
}
pom::GraphicsPipelineBuilder& pom::GraphicsPipelineBuilder::SetFrontFace(VkFrontFace front)
{
	m_RasterizerInfo.frontFace = front;
	return *this;
}

// Depth Stencil Info
pom::GraphicsPipelineBuilder& pom::GraphicsPipelineBuilder::SetDepthTest(VkBool32 depthRead, VkBool32 depthWrite, VkCompareOp compareOp)
{
	m_DepthStencilInfo.depthWriteEnable = depthWrite;
	m_DepthStencilInfo.depthTestEnable = depthRead;
	m_DepthStencilInfo.depthCompareOp = compareOp;
	return *this;
}

// Dynamic States Info
pom::GraphicsPipelineBuilder& pom::GraphicsPipelineBuilder::AddDynamicState(VkDynamicState dynamicState)
{
	m_vDynamicStates.push_back(dynamicState);
	m_DynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(m_vDynamicStates.size());
	m_DynamicStateInfo.pDynamicStates = m_vDynamicStates.data();
	return *this;
}

// Pipeline & Render Pass
pom::GraphicsPipelineBuilder& pom::GraphicsPipelineBuilder::SetPipelineLayout(const GraphicsPipelineLayout& layout)
{
	m_PipelineLayout = layout.GetLayout();
	return *this;
}
pom::GraphicsPipelineBuilder& pom::GraphicsPipelineBuilder::SetRenderPass(const RenderPass& renderPass)
{
	m_RenderPass = renderPass.GetRenderPass();
	return *this;
}

// Build
void pom::GraphicsPipelineBuilder::Build(const Device& device, GraphicsPipeline& pipeline) const
{
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(m_vShaderInfo.size());
	pipelineInfo.pStages = m_vShaderInfo.data();
	pipelineInfo.pVertexInputState = &m_VertexInputInfo;
	pipelineInfo.pInputAssemblyState = &m_InputAssembly;
	pipelineInfo.pViewportState = &m_ViewportState;
	pipelineInfo.pRasterizationState = &m_RasterizerInfo;
	pipelineInfo.pMultisampleState = &m_MultiSamplingInfo;
	pipelineInfo.pDepthStencilState = &m_DepthStencilInfo;
	pipelineInfo.pColorBlendState = &m_ColorBlendCreateInfo;
	pipelineInfo.pDynamicState = &m_DynamicStateInfo;
	pipelineInfo.layout = m_PipelineLayout;
	pipelineInfo.renderPass = m_RenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.m_Pipeline) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Graphics Pipeline!");
}
