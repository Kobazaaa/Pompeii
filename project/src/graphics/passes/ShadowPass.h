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
#include "Scene.h"
#include "RenderPass.h"
#include "Sampler.h"

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Create Info	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct ShadowPassCreateInfo
	{
		uint32_t maxFramesInFlight{};
		glm::vec<2, uint32_t> extent{};
		DescriptorPool* pDescriptorPool{};
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
		void Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, Scene* pScene);


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const Sampler& GetSampler() const;
		const Image& GetMap(uint32_t idx) const;

		//--------------------------------------------------
		//    Shader Infos
		//--------------------------------------------------
		struct alignas(16) LightDataVS
		{
			glm::mat4 lightSpace;
		};
		struct PCModelDataVS
		{
			glm::mat4 model;
		};

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