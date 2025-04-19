#ifndef SHADOW_PASS_H
#define SHADOW_PASS_H

// -- Math Includes --
#include "glm/glm.hpp"

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "DescriptorSet.h"
#include "FrameBuffer.h"
#include "GraphicsPipeline.h"
#include "Image.h"
#include "Model.h"
#include "RenderPass.h"
#include "Sampler.h"

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Shader Infos	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct LightData
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Create Info	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct ShadowPassCreateInfo
	{
		uint32_t maxFramesInFlight{};
		glm::vec<2, uint32_t> extent{};
		DescriptorPool* descriptorPool{};
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Shadow Pass	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class ShadowPass final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		void Initialize(const Context& context, const ShadowPassCreateInfo& createInfo);
		void Destroy();
		void Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, const Model& model);


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const Sampler& GetSampler() const;
		const Image& GetMap(uint32_t idx) const;

	private:
		// -- Pass --
		RenderPass					m_ShadowPass			{ };

		// -- Pipeline --
		GraphicsPipelineLayout		m_ShadowPipelineLayout	{ };
		GraphicsPipeline			m_ShadowPipeline		{ };

		// -- Image --
		std::vector<Image>			m_vShadowMaps			{ };
		Sampler						m_ShadowSampler			{ };

		// -- Descriptors --
		DescriptorSetLayout			m_LightDataDSL			{ };
		std::vector<DescriptorSet>	m_vLightDataDS			{ };

		// -- Buffers --
		std::vector<FrameBuffer>	m_vFrameBuffers			{ };
		std::vector<Buffer>			m_vLightDataBuffers		{ };

		// -- DQ --
		DeletionQueue				m_DeletionQueue			{ };
	};
}
#endif // SHADOW_PASS_H