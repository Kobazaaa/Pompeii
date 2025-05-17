#ifndef DEPTH_PRE_PASS_H
#define DEPTH_PRE_PASS_H

// -- Math Includes --
#include "glm/glm.hpp"

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "DescriptorSet.h"
#include "GraphicsPipeline.h"
#include "Sampler.h"
#include "Image.h"

// -- Forward Declarations --
namespace pom
{
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
	struct DepthPrePassCreateInfo
	{
		uint32_t maxFramesInFlight{};
		VkFormat depthFormat{};
		DescriptorPool* pDescriptorPool{};
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Depth PrePass	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class DepthPrePass final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		void Initialize(const Context& context, const DepthPrePassCreateInfo& createInfo);
		void Destroy();
		void Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, const Image& depthImage, Scene* pScene, Camera* pCamera) const;

		//--------------------------------------------------
		//    Shader Infos
		//--------------------------------------------------
		struct alignas(16) UniformBufferVS
		{
			glm::mat4 view;
			glm::mat4 proj;
		};
		struct PCModelDataVS
		{
			glm::mat4 model;
		};

	private:
		// -- Pipeline --
		GraphicsPipelineLayout		m_PipelineLayout{ };
		GraphicsPipeline			m_Pipeline{ };

		// -- Descriptors --
		DescriptorSetLayout			m_UniformDSL{ };
		std::vector<DescriptorSet>	m_vUniformDS{ };

		// -- Buffers --
		std::vector<Buffer>			m_vUniformBuffers{ };

		// -- DQ --
		DeletionQueue				m_DeletionQueue{ };
	};
}

#endif // DEPTH_PRE_PASS_H