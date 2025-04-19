#ifndef FORWARD_PASS_H
#define FORWARD_PASS_H

// -- Math Includes --
#include "glm/glm.hpp"

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "DescriptorSet.h"
#include "GraphicsPipeline.h"
#include "RenderPass.h"
#include "Sampler.h"

// -- Forward Declarations --
namespace pom
{
	class SwapChain;
	class DescriptorPool;
	struct Model;
	class ShadowPass;
	class FrameBuffer;
	class Camera;
	class CommandBuffer;
}

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Create Info	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct ForwardPassCreateInfo
	{
		uint32_t maxFramesInFlight{};
		Model* model{};
		SwapChain* swapChain{};
		DescriptorPool* descriptorPool{};
		ShadowPass* shadowPass{};
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Forward Pass	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class ForwardPass final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		void Initialize(const Context& context, const ForwardPassCreateInfo& createInfo);
		void Destroy();
		void Record(const Context& context, const FrameBuffer& fb, const SwapChain& sc, CommandBuffer& commandBuffer, uint32_t imageIndex, const Model& model, Camera* pCamera);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const RenderPass& GetRenderPass() const;


		//--------------------------------------------------
		//    Shader Infos
		//--------------------------------------------------
		struct alignas(16) UniformBufferVS
		{
			glm::mat4 view;
			glm::mat4 proj;
			glm::mat4 viewL;
			glm::mat4 projL;
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
			uint32_t specularIdx;
			uint32_t shininessIdx;
			uint32_t heightIdx;

			// -- Data --
			float expo;
		};

	private:
		// -- Pass --
		RenderPass					m_ForwardPass			{ };

		// -- Pipeline --
		GraphicsPipelineLayout		m_DefaultPipelineLayout	{ };
		GraphicsPipeline			m_OpaquePipeline		{ };
		GraphicsPipeline			m_TransparentPipeline	{ };

		// -- Image --
		Sampler						m_Sampler				{ };

		// -- Descriptors --
		DescriptorSetLayout			m_UniformDSL			{ };
		std::vector<DescriptorSet>	m_vUniformDS			{ };

		DescriptorSetLayout			m_ShadowMapDSL			{ };
		std::vector<DescriptorSet>	m_ShadowMapDS			{ };

		DescriptorSetLayout			m_TextureDSL			{ };
		DescriptorSet				m_TextureDS				{ };

		// -- Buffers --
		std::vector<Buffer>			m_vUniformBuffers		{ };

		// -- DQ --
		DeletionQueue				m_DeletionQueue			{ };
	};
}

#endif // FORWARD_PASS_H