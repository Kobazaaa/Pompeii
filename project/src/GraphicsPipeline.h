#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <vector>

// -- Forward Declarations --
namespace pom
{
	class RenderPass;
	class ShaderModule;
	class DescriptorSetLayout;
	struct Context;
}

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
		void Destroy(const Context& context) const;


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkPipelineLayout& GetHandle() const;

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

		void Build(const Context& context, GraphicsPipelineLayout& pipelineLayout);
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
		void Destroy(const Context& context) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkPipeline& GetHandle() const;

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

		// Debug Info
		GraphicsPipelineBuilder& SetDebugName(const char* name);

		// Shader Info
		GraphicsPipelineBuilder& AddShader(const ShaderModule& shader, VkShaderStageFlagBits stage);
		GraphicsPipelineBuilder& SetShaderSpecialization(uint32_t constID, uint32_t offset, uint32_t size, const void* data);


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

		void Build(const Context& context, GraphicsPipeline& pipeline) const;

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
		const char*			m_pName{};

		std::vector<VkDynamicState> m_vDynamicStates;
		std::vector<VkPipelineShaderStageCreateInfo> m_vShaderInfo;
		std::vector<VkSpecializationMapEntry > m_vShaderSpecializationEntries;
		std::vector<VkSpecializationInfo> m_vSpecializationInfo;
	};
}

#endif // GRAPHICS_PIPELINE_H