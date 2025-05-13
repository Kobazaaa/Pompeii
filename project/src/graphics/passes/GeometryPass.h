#ifndef GEOMETRY_PASS_H
#define GEOMETRY_PASS_H

// -- Math Includes --
#include "glm/glm.hpp"

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "DescriptorSet.h"
#include "GBuffer.h"
#include "GraphicsPipeline.h"
#include "Sampler.h"
#include "Image.h"

// -- Forward Declarations --
namespace pom
{
	class DescriptorPool;
	struct Scene;
	class Camera;
	class CommandBuffer;
}

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Create Info	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct GeometryPassCreateInfo
	{
		uint32_t maxFramesInFlight{};
		Scene* pScene{};
		VkExtent2D extent{};
		VkFormat depthFormat{};
		DescriptorPool* pDescriptorPool{};
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Geometry Pass	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class GeometryPass final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		void Initialize(const Context& context, const GeometryPassCreateInfo& createInfo);
		void Destroy();
		void Resize(const Context& context, VkExtent2D extent);
		void Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, Image& depthImage, Scene* pScene, Camera* pCamera);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const std::vector<GBuffer>& GetGBuffers() const;
		const GBuffer& GetGBuffer(uint32_t index) const;

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

		struct PCMaterialDataFS
		{
			// -- Textures --
			uint32_t diffuseIdx;
			uint32_t opacityIdx;
			uint32_t normalIdx;
			uint32_t roughnessIdx;
			uint32_t metallicIdx;
		};

	private:
		// -- Pipeline --
		GraphicsPipelineLayout		m_PipelineLayout{ };
		GraphicsPipeline			m_Pipeline{ };

		// -- Descriptors --
		DescriptorSetLayout			m_UniformDSL{ };
		std::vector<DescriptorSet>	m_vUniformDS{ };

		DescriptorSetLayout			m_TextureDSL{ };
		DescriptorSet				m_TextureDS{ };

		// -- Buffers --
		std::vector<GBuffer>		m_vGBuffers;
		std::vector<Buffer>			m_vUniformBuffers;

		Sampler						m_TextureSampler{ };

		DeletionQueue				m_DeletionQueue{ };
	};
}

#endif // GEOMETRY_PASS_H
