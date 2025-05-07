#ifndef LIGHTING_PASS_H
#define LIGHTING_PASS_H

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "DescriptorPool.h"
#include "GraphicsPipeline.h"
#include "Sampler.h"
#include "DescriptorSet.h"

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
		VkFormat depthFormat{};
		DescriptorPool* pDescriptorPool{};
		GeometryPass* pGeometryPass;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Lighting Pass	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class LightingPass
	{
	public:
		void Initialize(const Context& context, const LightingPassCreateInfo& createInfo);
		void Destroy();
		void Resize(const Context& context, const GeometryPass& pGeometryPass) const;
		void Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, Image& recordImage, Image& depthImage, Scene* pScene, Camera* pCamera);

	private:
		// -- Pipeline --
		GraphicsPipelineLayout		m_PipelineLayout		{ };
		GraphicsPipeline			m_Pipeline				{ };

		// -- Image --
		Sampler						m_GBufferSampler		{ };

		// -- Descriptors --
		DescriptorSetLayout			m_GBufferTexturesDSL	{ };
		std::vector<DescriptorSet>	m_vGBufferTexturesDS	{ };

		// -- DQ --
		DeletionQueue				m_DeletionQueue			{ };
	};
}

#endif // LIGHTING_PASS_H