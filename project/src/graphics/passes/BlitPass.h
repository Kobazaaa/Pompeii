#ifndef BLIT_PASS_H
#define BLIT_PASS_H

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "DescriptorPool.h"
#include "GraphicsPipeline.h"
#include "Sampler.h"
#include "DescriptorSet.h"

// -- Forward Declarations --
namespace pom
{
	class LightingPass;
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
	struct BlitPassCreateInfo
	{
		uint32_t maxFramesInFlight{};
		VkFormat format{};
		DescriptorPool* pDescriptorPool{};
		std::vector<Image>* renderImages{};
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Lighting Pass	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class BlitPass
	{
	public:
		void Initialize(const Context& context, const BlitPassCreateInfo& createInfo);
		void Destroy();
		void UpdateDescriptors(const Context& context, const std::vector<Image>& renderImages) const;
		void Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, const Image& renderImage) const;

	private:
		// -- Pipeline --
		GraphicsPipelineLayout		m_PipelineLayout{ };
		GraphicsPipeline			m_Pipeline{ };

		// -- Image --
		Sampler						m_Sampler{ };

		// -- Descriptors --
		DescriptorSetLayout			m_TextureDSL{ };
		std::vector<DescriptorSet>	m_vTexturesDS{ };

		// -- DQ --
		DeletionQueue				m_DeletionQueue{ };
	};
}

#endif // BLIT_PASS_H