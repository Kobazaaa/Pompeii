#ifndef LIGHTING_PASS_H
#define LIGHTING_PASS_H

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "Pipeline.h"
#include "Sampler.h"
#include "DescriptorSet.h"
#include "Buffer.h"
#include "Light.h"

// -- Forward Declarations --
namespace pompeii
{
	struct LightItem;
	class GeometryPass;
	class DescriptorPool;
	class EnvironmentMap;
	struct CameraData;
	class CommandBuffer;
}

namespace pompeii
{	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Create Info	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct LightingPassCreateInfo
	{
		VkFormat format{};
		GeometryPass* pGeometryPass;
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
		void UpdateGBufferDescriptors(const Context& context, const GeometryPass& pGeometryPass, const std::vector<Image>& depthImages) const;
		void UpdateEnvironmentMap(const Context& context, const EnvironmentMap& envMap) const;
		void UpdateLightData(const Context& context, const std::vector<Light*>& data);
		void UpdateShadowMaps(const Context& context, const std::vector<LightItem>& lightItems);
		void Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, const Image& renderImage, const CameraData& camera) const;

	private:
		// -- Pipeline --
		PipelineLayout				m_PipelineLayout		{ };
		Pipeline					m_Pipeline				{ };

		// -- Image --
		Sampler						m_GBufferSampler		{ };
		Sampler						m_ShadowSampler			{ };

		// -- Descriptors --
		DescriptorSetLayout			m_CameraMatricesDSL		{ };
		DescriptorSetLayout			m_SSBOLightDSL			{ };
		DescriptorSetLayout			m_UBOLightMapDSL		{ };
		DescriptorSetLayout			m_GBufferTexturesDSL	{ };

		std::vector<DescriptorSet>	m_vCameraMatricesDS		{ };
		DescriptorSet				m_SSBOLightDS			{ };
		std::vector<DescriptorSet>	m_vUBODirLightMapDS		{ };
		std::vector<DescriptorSet>	m_vUBOPointLightMapDS	{ };
		std::vector<DescriptorSet>	m_vGBufferTexturesDS	{ };

		std::vector<Buffer>			m_vCameraMatrices		{ };
		Buffer						m_SSBOLights			{ };

		// -- DQ --
		DeletionQueue				m_DeletionQueue			{ };
	};
}

#endif // LIGHTING_PASS_H