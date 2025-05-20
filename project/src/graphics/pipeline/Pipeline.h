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
	//? ~~	  PipelineLayout	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class PipelineLayout final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit PipelineLayout() = default;
		~PipelineLayout() = default;
		PipelineLayout(const PipelineLayout& other) = delete;
		PipelineLayout(PipelineLayout&& other) noexcept = delete;
		PipelineLayout& operator=(const PipelineLayout& other) = delete;
		PipelineLayout& operator=(PipelineLayout&& other) noexcept = delete;
		void Destroy(const Context& context) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkPipelineLayout& GetHandle() const;

	private:
		VkPipelineLayout m_Layout;
		friend class PipelineLayoutBuilder;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  GraphicsPipelineLayoutBuilder	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class PipelineLayoutBuilder final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		PipelineLayoutBuilder();

		//--------------------------------------------------
		//    Builder
		//--------------------------------------------------

		//? Optional
		PipelineLayoutBuilder& NewPushConstantRange();
		//! REQUIRED IF PC
		PipelineLayoutBuilder& SetPCStageFlags(VkShaderStageFlags flags);
		// If not called, default of 0 assumed
		PipelineLayoutBuilder& SetPCOffset(uint32_t offset);
		//! REQUIRED IF PC
		PipelineLayoutBuilder& SetPCSize(uint32_t size);

		//! REQUIRED
		PipelineLayoutBuilder& AddLayout(const pom::DescriptorSetLayout& descriptorSetLayout);

		void Build(const Context& context, PipelineLayout& pipelineLayout);
	private:
		VkPipelineLayoutCreateInfo m_PipelineLayoutInfo{};
		std::vector<VkPushConstantRange> m_vPushConstantRanges;
		std::vector<VkDescriptorSetLayout> m_vDescriptorLayouts;
	};



	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Pipeline
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Pipeline final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit Pipeline() = default;
		~Pipeline() = default;
		Pipeline(const Pipeline& other) = delete;
		Pipeline(Pipeline&& other) noexcept = delete;
		Pipeline& operator=(const Pipeline& other) = delete;
		Pipeline& operator=(Pipeline&& other) noexcept = delete;
		void Destroy(const Context& context) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkPipeline& GetHandle() const;

	private:
		VkPipeline m_Pipeline;
		friend class GraphicsPipelineBuilder;
		friend class ComputePipelineBuilder;
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
		GraphicsPipelineBuilder& SetPipelineLayout(const PipelineLayout& layout);
		// If not set, it is assumed dynamic rendering is taking place
		GraphicsPipelineBuilder& SetRenderPass(const RenderPass& renderPass);
		// If not set, it is assumed render pass rendering is taking place
		GraphicsPipelineBuilder& SetupDynamicRendering(VkPipelineRenderingCreateInfo& dynamicRenderInfo);

		void Build(const Context& context, Pipeline& pipeline);

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

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  ComputePipelineBuilder	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class ComputePipelineBuilder final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		ComputePipelineBuilder();

		//--------------------------------------------------
		//    Builder
		//--------------------------------------------------
		// Debug Info
		ComputePipelineBuilder& SetDebugName(const char* name);

		// Shader Info
		ComputePipelineBuilder& SetShader(const ShaderModule& shader);
		ComputePipelineBuilder& SetShaderSpecialization(uint32_t constID, uint32_t offset, uint32_t size, const void* data);

		//! REQUIRED
		ComputePipelineBuilder& SetPipelineLayout(const PipelineLayout& layout);

		void Build(const Context& context, Pipeline& pipeline) const;

	private:
		VkPipelineLayout	m_PipelineLayout;
		const char*			m_pName;

		VkPipelineShaderStageCreateInfo m_ShaderInfo;
		VkSpecializationMapEntry m_ShaderSpecializationEntry;
		VkSpecializationInfo m_SpecializationInfo;
	};
}

#endif // GRAPHICS_PIPELINE_H