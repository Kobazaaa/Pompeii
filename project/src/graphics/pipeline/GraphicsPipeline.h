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
		explicit GraphicsPipelineLayout() = default;
		~GraphicsPipelineLayout() = default;
		GraphicsPipelineLayout(const GraphicsPipelineLayout& other) = delete;
		GraphicsPipelineLayout(GraphicsPipelineLayout&& other) noexcept = delete;
		GraphicsPipelineLayout& operator=(const GraphicsPipelineLayout& other) = delete;
		GraphicsPipelineLayout& operator=(GraphicsPipelineLayout&& other) noexcept = delete;
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
		GraphicsPipelineLayoutBuilder& SetPCStageFlags(VkShaderStageFlags flags);
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
		explicit GraphicsPipeline() = default;
		~GraphicsPipeline() = default;
		GraphicsPipeline(const GraphicsPipeline& other) = delete;
		GraphicsPipeline(GraphicsPipeline&& other) noexcept = delete;
		GraphicsPipeline& operator=(const GraphicsPipeline& other) = delete;
		GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept = delete;
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
		// If not set, disabled by default
		GraphicsPipelineBuilder& EnableDepthBias(float constantFactor, float slopeFactor);

		// Multi Sampling Info
		// If not set, disabled by default
		GraphicsPipelineBuilder& EnableSampleShading(float minSampleShading);
		// If not set, sample count set to VK_SAMPLE_COUNT_1_BIT
		GraphicsPipelineBuilder& SetSampleCount(VkSampleCountFlagBits samples);


		// Depth Stencil Info
		// If not set, enabled by default with VK_COMPARE_OP_LESS with WRITE and READ Enabled
		GraphicsPipelineBuilder& SetDepthTest(VkBool32 depthRead, VkBool32 depthWrite, VkCompareOp compareOp);

		// Blend Info
		// Off by default
		GraphicsPipelineBuilder& EnableBlend(uint32_t attachment);
		// If not set, RGBA assumed by default
		GraphicsPipelineBuilder& SetColorWriteMask(VkColorComponentFlags colorComponents, uint32_t attachment);
		// If not set, ONE, ZERO, ADD assumed by default
		GraphicsPipelineBuilder& SetColorBlend(VkBlendFactor src, VkBlendFactor dst, VkBlendOp op);
		// If not set, ONE, ZERO, ADD assumed by default
		GraphicsPipelineBuilder& SetAlphaBlend(VkBlendFactor src, VkBlendFactor dst, VkBlendOp op);

		// Dynamic States
		GraphicsPipelineBuilder& AddDynamicState(VkDynamicState dynamicState);

		//! REQUIRED
		GraphicsPipelineBuilder& SetPipelineLayout(const GraphicsPipelineLayout& layout);
		// If not set, it is assumed dynamic rendering is taking place
		GraphicsPipelineBuilder& SetRenderPass(const RenderPass& renderPass);
		// If not set, it is assumed render pass rendering is taking place
		GraphicsPipelineBuilder& SetupDynamicRendering(VkPipelineRenderingCreateInfo& dynamicRenderInfo);

		void Build(const Context& context, GraphicsPipeline& pipeline);

	private:
		// wtf vulkan
		VkPipelineVertexInputStateCreateInfo				m_VertexInputInfo{};
		VkPipelineInputAssemblyStateCreateInfo				m_InputAssembly{};
		VkPipelineViewportStateCreateInfo					m_ViewportState{};
		VkPipelineRasterizationStateCreateInfo				m_RasterizerInfo{};
		VkPipelineMultisampleStateCreateInfo				m_MultiSamplingInfo{};
		VkPipelineDepthStencilStateCreateInfo				m_DepthStencilInfo{};
		std::vector<VkPipelineColorBlendAttachmentState>	m_vColorBlendAttachmentState{};
		VkPipelineColorBlendStateCreateInfo					m_ColorBlendCreateInfo{};
		VkPipelineDynamicStateCreateInfo					m_DynamicStateInfo{};

		VkPipelineLayout	m_PipelineLayout;
		VkRenderPass		m_RenderPass;
		void*				m_pNext;
		const char*			m_pName{};
		uint32_t			m_CurrentAttachment{};

		std::vector<VkDynamicState> m_vDynamicStates;
		std::vector<VkPipelineShaderStageCreateInfo> m_vShaderInfo;
		std::vector<VkSpecializationMapEntry > m_vShaderSpecializationEntries;
		std::vector<VkSpecializationInfo> m_vSpecializationInfo;
	};
}

#endif // GRAPHICS_PIPELINE_H