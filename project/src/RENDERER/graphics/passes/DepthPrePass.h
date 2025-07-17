#ifndef DEPTH_PRE_PASS_H
#define DEPTH_PRE_PASS_H

// -- Math Includes --
#include "glm/glm.hpp"

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "DescriptorSet.h"
#include "Pipeline.h"
#include "Image.h"

// -- Forward Declarations --
namespace pompeii
{
	class GeometryPass;
	class DescriptorPool;
	class Scene;
	class Camera;
	class CommandBuffer;
	struct RenderDrawContext;
	struct RenderInstance;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Create Info	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct DepthPrePassCreateInfo
	{
		uint32_t maxFramesInFlight{};
		VkFormat depthFormat{};
		GeometryPass* pGeometryPass{};
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
		explicit DepthPrePass() = default;
		~DepthPrePass() = default;
		DepthPrePass(const DepthPrePass& other) = delete;
		DepthPrePass(DepthPrePass&& other) noexcept = delete;
		DepthPrePass& operator=(const DepthPrePass& other) = delete;
		DepthPrePass& operator=(DepthPrePass&& other) noexcept = delete;

		void Initialize(const Context& context, const DepthPrePassCreateInfo& createInfo);
		void Destroy();
		void UpdateCamera(const Context& context, uint32_t imageIndex, Camera* pCamera) const;
		void Record(CommandBuffer& commandBuffer, const GeometryPass& gPass, uint32_t imageIndex, const Image& depthImage, const RenderDrawContext& renderContext) const;

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
		PipelineLayout		m_PipelineLayout{ };
		Pipeline			m_Pipeline{ };

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