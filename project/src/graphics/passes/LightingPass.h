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
#include "glm/vec4.hpp"

// -- Forward Declarations --
namespace pom
{
	class GeometryPass;
	class DescriptorPool;
	class Scene;
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
		explicit LightingPass() = default;
		~LightingPass() = default;
		LightingPass(const LightingPass& other) = delete;
		LightingPass(LightingPass&& other) noexcept = delete;
		LightingPass& operator=(const LightingPass& other) = delete;
		LightingPass& operator=(LightingPass&& other) noexcept = delete;

		void Initialize(const Context& context, const LightingPassCreateInfo& createInfo);
		void Destroy();
		void UpdateDescriptors(const Context& context, const GeometryPass& pGeometryPass) const;
		void Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, const Image& renderImage, Scene* pScene, Camera* pCamera) const;

		//--------------------------------------------------
		//    Shader Infos
		//--------------------------------------------------
		struct alignas(16) SSBOFrag
		{
			glm::vec4 dirPosType;
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
		DescriptorSetLayout			m_CameraMatricesDSL		{ };
		DescriptorSetLayout			m_SSBOLightDSL			{ };
		DescriptorSetLayout			m_GBufferTexturesDSL	{ };

		std::vector<DescriptorSet>	m_vCameraMatricesDS		{ };
		std::vector<DescriptorSet>	m_vSSBOLightDS			{ };
		std::vector<DescriptorSet>	m_vGBufferTexturesDS	{ };

		std::vector<Buffer>			m_vCameraMatrices		{ };
		std::vector<Buffer>			m_vSSBOLights			{ };

		// -- DQ --
		DeletionQueue				m_DeletionQueue			{ };
	};
}

#endif // LIGHTING_PASS_H