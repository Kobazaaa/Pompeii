#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H
#include "DescriptorSet.h"
#include "Device.h"
#include "RenderPass.h"
#include "Shader.h"

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  GraphicsPipelineLayout	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class GraphicsPipelineLayout final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		GraphicsPipelineLayout() = default;
		void Destroy(const Device& device) const;


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkPipelineLayout& GetLayout() const;

	private:
		VkPipelineLayout m_Layout;
		friend class GraphicsPipelineLayoutBuilder;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  GraphicsPipelineLayoutBuilder	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class GraphicsPipelineLayoutBuilder final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		GraphicsPipelineLayoutBuilder();


		//--------------------------------------------------
		//    Builder
		//--------------------------------------------------

		//? Optional
		GraphicsPipelineLayoutBuilder& NewPushConstantRange();
		//! REQUIRED IF PC
		GraphicsPipelineLayoutBuilder& SetPCStageFlags(VkPipelineStageFlags flags);
		// If not called, default of 0 assumed
		GraphicsPipelineLayoutBuilder& SetPCOffset(uint32_t offset);
		//! REQUIRED IF PC
		GraphicsPipelineLayoutBuilder& SetPCSize(uint32_t size);

		//! REQUIRED
		GraphicsPipelineLayoutBuilder& AddLayout(const pom::DescriptorSetLayout& descriptorSetLayout);

		void Build(const Device& device, GraphicsPipelineLayout& pipelineLayout);
	private:
		VkPipelineLayoutCreateInfo m_PipelineLayoutInfo{};
		std::vector<VkPushConstantRange> m_vPushConstantRanges;
		std::vector<VkDescriptorSetLayout> m_vDescriptorLayouts;
	};



	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  GraphicsPipeline
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class GraphicsPipeline final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		GraphicsPipeline() = default;
		void Destroy(const Device& device) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkPipeline& GetPipeline() const;

	private:
		VkPipeline m_Pipeline;
		friend class GraphicsPipelineBuilder;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  GraphicsPipelineBuilder	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class GraphicsPipelineBuilder final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		GraphicsPipelineBuilder();

		//--------------------------------------------------
		//    Builder
		//--------------------------------------------------

		// Shader Info
		GraphicsPipelineBuilder& SetShader(const ShaderModule& shader, VkShaderStageFlagBits stage);

		// Vertex Input Info
		//! REQUIRED
		GraphicsPipelineBuilder& SetVertexBindingDesc(const VkVertexInputBindingDescription& desc);
		//! REQUIRED
		GraphicsPipelineBuilder& SetVertexAttributeDesc(const std::vector<VkVertexInputAttributeDescription>& attr);

		// Input Assembly Info
		//! REQUIRED
		GraphicsPipelineBuilder& SetPrimitiveTopology(VkPrimitiveTopology topology);

		// Rasterizer Info
		//! REQUIRED
		GraphicsPipelineBuilder& SetCullMode(VkCullModeFlags cullMode);
		//! REQUIRED
		GraphicsPipelineBuilder& SetPolygonMode(VkPolygonMode polyMode);
		// If not set, default of VK_FRONT_FACE_CLOCKWISE assumed
		GraphicsPipelineBuilder& SetFrontFace(VkFrontFace front);

		// Depth Stencil Info
		// If not set, enabled by default with VK_COMPARE_OP_LESS
		GraphicsPipelineBuilder& SetDepthTest(VkBool32 depthRead, VkBool32 depthWrite, VkCompareOp compareOp);

		// Dynamic States
		GraphicsPipelineBuilder& AddDynamicState(VkDynamicState dynamicState);

		//! REQUIRED
		GraphicsPipelineBuilder& SetPipelineLayout(const GraphicsPipelineLayout& layout);
		//! REQUIRED
		GraphicsPipelineBuilder& SetRenderPass(const RenderPass& renderPass);

		void Build(const Device& device, GraphicsPipeline& pipeline) const;

	private:
		// wtf vulkan
		VkPipelineVertexInputStateCreateInfo	m_VertexInputInfo{};
		VkPipelineInputAssemblyStateCreateInfo	m_InputAssembly{};
		VkPipelineViewportStateCreateInfo		m_ViewportState{};
		VkPipelineRasterizationStateCreateInfo	m_RasterizerInfo{};
		VkPipelineMultisampleStateCreateInfo	m_MultiSamplingInfo{};
		VkPipelineDepthStencilStateCreateInfo	m_DepthStencilInfo{};
		VkPipelineColorBlendAttachmentState		m_ColorBlendAttachmentState{};
		VkPipelineColorBlendStateCreateInfo		m_ColorBlendCreateInfo{};
		VkPipelineDynamicStateCreateInfo		m_DynamicStateInfo{};

		VkPipelineLayout	m_PipelineLayout;
		VkRenderPass		m_RenderPass;

		std::vector<VkDynamicState> m_vDynamicStates;
		std::vector<VkPipelineShaderStageCreateInfo> m_vShaderInfo;
	};
}

#endif // GRAPHICS_PIPELINE_H