#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H
#include "DescriptorSet.h"
#include "Device.h"

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
}

#endif // GRAPHICS_PIPELINE_H