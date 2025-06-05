#ifndef LIGHTING_PASS_H
#define LIGHTING_PASS_H

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "DescriptorPool.h"
#include "Pipeline.h"
#include "Sampler.h"
#include "DescriptorSet.h"
#include "Buffer.h"

// -- Math Includes --
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

// -- Forward Declarations --
namespace pom
{
	class GeometryPass;
	class DescriptorPool;
	class Scene;
	class EnvironmentMap;
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
		GeometryPass* pGeometryPass;
		Scene* pScene;
		std::vector<Image>* pDepthImages;
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
		void UpdateGBufferDescriptors(const Context& context, const GeometryPass& pGeometryPass, const std::vector<Image>& depthImages, const EnvironmentMap& envMap) const;
		void UpdateLightDescriptors(const Context& context, Scene* pScene);
		void Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, const Image& renderImage, Scene* pScene, Camera* pCamera) const;

	private:
		// -- Pipeline --
		PipelineLayout		m_PipelineLayout		{ };
		Pipeline			m_Pipeline				{ };

		// -- Image --
		Sampler						m_GBufferSampler		{ };
		Sampler						m_ShadowSampler			{ };

		// -- Descriptors --
		DescriptorSetLayout			m_CameraMatricesDSL		{ };
		DescriptorSetLayout			m_SSBOLightDSL			{ };
		DescriptorSetLayout			m_SSBOLightMatricesDSL	{ };
		DescriptorSetLayout			m_UBOLightMapDSL		{ };
		DescriptorSetLayout			m_GBufferTexturesDSL	{ };

		std::vector<DescriptorSet>	m_vCameraMatricesDS		{ };
		DescriptorSet				m_SSBOLightDS			{ };
		DescriptorSet				m_SSBOLightMatricesDS	{ };
		DescriptorSet				m_UBODirLightMapDS		{ };
		DescriptorSet				m_UBOPointLightMapDS	{ };
		std::vector<DescriptorSet>	m_vGBufferTexturesDS	{ };

		std::vector<Buffer>			m_vCameraMatrices		{ };
		Buffer						m_SSBOLights			{ };
		Buffer						m_SSBOLightsMatrices	{ };

		// -- DQ --
		DeletionQueue				m_DeletionQueue			{ };
	};
}

#endif // LIGHTING_PASS_H