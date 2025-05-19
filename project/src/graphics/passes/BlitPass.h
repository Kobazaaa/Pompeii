#ifndef BLIT_PASS_H
#define BLIT_PASS_H

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "GraphicsPipeline.h"
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
		void Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, const Image& renderImage, const Camera* pCamera) const;

	private:
		// -- Pipeline --
		GraphicsPipelineLayout		m_PipelineLayout{ };
		GraphicsPipeline			m_Pipeline{ };

		// -- Image --
		Sampler						m_Sampler{ };

		// -- Descriptors --
		DescriptorSetLayout			m_FragmentDSL{ };
		std::vector<DescriptorSet>	m_vFragmentDS{ };
		std::vector<Buffer>			m_vCameraSettings{ };

		// -- DQ --
		DeletionQueue				m_DeletionQueue{ };
	};
}

#endif // BLIT_PASS_H