#ifndef RENDERER_H
#define RENDERER_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Math Includes --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtc/quaternion.hpp"

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "Context.h"
#include "CommandPool.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "FrameBuffer.h"
#include "GraphicsPipeline.h"
#include "Window.h"
#include "Model.h"
#include "RenderPass.h"
#include "Sampler.h"
#include "SwapChain.h"
#include "SyncManager.h"


// -- Forward Declarations --
namespace pom
{
	class Window;
	class Camera;
}

// -- UBO --
struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(16) glm::vec3 cam;
};
struct LightUBO
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(16) glm::vec3 cam;
};

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Renderer	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Renderer final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		Renderer() = default;
		void Initialize(Camera* pCamera, Window* pWindow);
		void Destroy();

		//--------------------------------------------------
		//    Loop
		//--------------------------------------------------
		void Update();
		void Render();

	private:
		//--------------------------------------------------
		//    Vulkan Specific
		//--------------------------------------------------
		void InitializeVulkan();

		// -- Vulkan Context --
		Context						m_Context				{ };

		// -- SwapChain --
		SwapChain					m_SwapChain				{ };
		std::vector<FrameBuffer>	m_vFrameBuffers			{ };
		uint32_t					m_MaxFramesInFlight		{ 3 };
		Image						m_MSAAImage				{ };

		// -- Model --
		Sampler						m_TextureSampler		{ };
		Model						m_Model					{ };

		// -- Renderer --
		RenderPass					m_RenderPass			{ };
		GraphicsPipelineLayout		m_PipelineLayout		{ };
		GraphicsPipeline			m_GraphicsPipeline		{ };
		GraphicsPipeline			m_TransPipeline			{ };

		// -- Descriptors
		DescriptorPool				m_DescriptorPool		{ };

		DescriptorSetLayout			m_LightMapDSL			{ };
		std::vector<DescriptorSet>	m_LightMapDS			{ };
		DescriptorSetLayout			m_UniformDSL			{ };
		std::vector<DescriptorSet>	m_vUniformDS			{ };
		std::vector<Buffer>			m_vUniformBuffers		{ };

		DescriptorSetLayout			m_TextureDSL			{ };
		DescriptorSet				m_TextureDS				{ };

		// -- Command --
		CommandPool					m_CommandPool			{ };
		SyncManager					m_SyncManager			{ };

		// -- Shadow Pass --
		RenderPass					m_ShadowPass{ };
		GraphicsPipelineLayout		m_ShadowPipelineLayout{ };
		GraphicsPipeline			m_ShadowPipeline{ };
		std::vector<Image>			m_vShadowMaps{ };
		Sampler						m_ShadowSampler{ };
		std::vector<FrameBuffer>	m_vShadowFrameBuffers{ };
		std::vector<DescriptorSet>	m_vLightDS	{ };
		std::vector<Buffer>			m_vLightBuffers{ };


		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void RecreateSwapChain();
		void CreateFrameBuffers();
		void LoadModels();
		void RecordCommandBuffer(CommandBuffer& commandBuffer, uint32_t imageIndex) const;
		void UpdateUniformBuffer(uint32_t currentImage) const;

		// -- Window --
		Window*				m_pWindow			{ };
		Camera*				m_pCamera			{ };

		// -- Frame Counter --
		uint32_t			m_CurrentFrame		{ 0 };

		// -- Deletion Queues --
		DeletionQueue	m_DeletionQueueSC	{ };

	};
}

#endif // RENDERER_H