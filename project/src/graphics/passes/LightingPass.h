#ifndef LIGHTING_PASS_H
#define LIGHTING_PASS_H

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "DescriptorPool.h"
#include "GraphicsPipeline.h"
#include "Sampler.h"
#include "DescriptorSet.h"

// -- Math Includes --
#include "glm/vec3.hpp"

// -- Forward Declarations --
namespace pom
{
	class GeometryPass;
	class DescriptorPool;
	struct Scene;
	class ShadowPass;
	class Camera;
	class CommandBuffer;
}

namespace pom
{	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Create Info	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct LightingPassCreateInfo
	{
		uint32_t maxFramesInFlight{};
		VkFormat format{};
		DescriptorPool* pDescriptorPool{};
		GeometryPass* pGeometryPass;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Lighting Pass	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class LightingPass
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		void Initialize(const Context& context, const LightingPassCreateInfo& createInfo);
		void Destroy();
		void UpdateDescriptors(const Context& context, const GeometryPass& pGeometryPass) const;
		void Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, const Image& renderImage, const Scene* pScene) const;

		//--------------------------------------------------
		//    Shader Infos
		//--------------------------------------------------
		struct alignas(16) UniformBufferFS
		{
			glm::vec3 dir;
			float padding;
			glm::vec3 color;
			float intensity;
		};

	private:
		// -- Pipeline --
		GraphicsPipelineLayout		m_PipelineLayout		{ };
		GraphicsPipeline			m_Pipeline				{ };

		// -- Image --
		Sampler						m_GBufferSampler		{ };

		// -- Descriptors --
		DescriptorSetLayout			m_GBufferTexturesDSL	{ };
		std::vector<DescriptorSet>	m_vGBufferTexturesDS	{ };

		DescriptorSetLayout			m_UniformDSL			{ };
		std::vector<DescriptorSet>	m_vUniformDS			{ };
		std::vector<Buffer>			m_vLightBuffers			{ };

		// -- DQ --
		DeletionQueue				m_DeletionQueue			{ };
	};
}

#endif // LIGHTING_PASS_H