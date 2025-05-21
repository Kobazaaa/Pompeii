#ifndef BLIT_PASS_H
#define BLIT_PASS_H

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "Pipeline.h"
#include "Sampler.h"
#include "DescriptorSet.h"

// -- Forward Declarations --
namespace pom
{
	class Camera;
	class DescriptorPool;
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
	//? ~~	  Blit Pass	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class BlitPass
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit BlitPass() = default;
		~BlitPass() = default;
		BlitPass(const BlitPass& other) = delete;
		BlitPass(BlitPass&& other) noexcept = delete;
		BlitPass& operator=(const BlitPass& other) = delete;
		BlitPass& operator=(BlitPass&& other) noexcept = delete;

		void Initialize(const Context& context, const BlitPassCreateInfo& createInfo);
		void Destroy();
		void UpdateDescriptors(const Context& context, const std::vector<Image>& renderImages) const;
		void RecordGraphic(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, const Image& renderImage, const Camera* pCamera);
		void RecordCompute(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, const Image& renderImage);

	private:
		// -- Pipeline --
		PipelineLayout				m_PipelineLayout{ };
		Pipeline					m_Pipeline{ };
		PipelineLayout				m_ComputePipelineLayout{ };
		Pipeline					m_CompPipeHistogram{ };
		Pipeline					m_CompPipeAverageLuminance{ };

		// -- Image --
		Sampler						m_Sampler{ };

		// -- Descriptors --
		// Fragment
		DescriptorSetLayout			m_FragmentDSL{ };
		std::vector<DescriptorSet>	m_vFragmentDS{ };
		std::vector<Buffer>			m_vCameraSettings{ };
		// Compute share
		DescriptorSetLayout			m_ComputeDSL{ };
		// Compute 1
		std::vector<DescriptorSet>	m_vComputeLumDS{ };
		std::vector<Buffer>			m_vHistogram{ };
		// Compute 2
		std::vector<DescriptorSet>	m_vComputeAveDS{ };
		std::vector<Image>			m_vAverageLuminance{ };

		// -- DQ --
		DeletionQueue				m_DeletionQueue{ };
	};
}

#endif // BLIT_PASS_H