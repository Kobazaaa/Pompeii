#ifndef SHADOW_PASS_H
#define SHADOW_PASS_H

// -- Math Includes --
#include "glm/glm.hpp"

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "DescriptorSet.h"
#include "Pipeline.h"
#include "Image.h"
#include "Scene.h"
#include "Sampler.h"

//! This Shadow Pass is NO LONGER IN USE!!!
//! Shadows are handled differently now!

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Create Info	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct ShadowPassCreateInfo
	{
		uint32_t maxFramesInFlight{};
		glm::vec<2, uint32_t> extent{};
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
		explicit ShadowPass() = default;
		~ShadowPass() = default;
		ShadowPass(const ShadowPass& other) = delete;
		ShadowPass(ShadowPass&& other) noexcept = delete;
		ShadowPass& operator=(const ShadowPass& other) = delete;
		ShadowPass& operator=(ShadowPass&& other) noexcept = delete;

		void Initialize(Context& context, const ShadowPassCreateInfo& createInfo);
		void Destroy();
		void Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, Scene* pScene);


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const Sampler& GetSampler() const;
		Image& GetMap(uint32_t idx);

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
		// -- Pipeline --
		PipelineLayout		m_ShadowPipelineLayout	{ };
		Pipeline			m_ShadowPipeline		{ };

		// -- Image --
		std::vector<Image>			m_vShadowMaps			{ };
		Sampler						m_ShadowSampler			{ };

		// -- Descriptors --
		DescriptorSetLayout			m_LightDataDSL			{ };
		std::vector<DescriptorSet>	m_vLightDataDS			{ };

		// -- Buffers --
		std::vector<Buffer>			m_vLightDataBuffers		{ };

		// -- DQ --
		DeletionQueue				m_DeletionQueue			{ };
	};
}
#endif // SHADOW_PASS_H