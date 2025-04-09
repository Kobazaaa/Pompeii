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
		uint32_t					m_MaxFramesInFlight		{ 2 };
		glm::vec4					m_ClearColor			{ 1.f, 0.f, 1.f, 1.f };

		// -- Model --
		Sampler						m_TextureSampler		{ };
		Model						m_Model					{ };

		// -- Renderer --
		RenderPass					m_RenderPass			{ };
		GraphicsPipelineLayout		m_PipelineLayout		{ };
		GraphicsPipeline			m_GraphicsPipeline		{ };

		// -- Descriptors
		std::vector<Buffer>			m_vUniformBuffers		{ };
		DescriptorSetLayout			m_UniformDSL			{ };
		DescriptorSetLayout			m_TextureDSL			{ };
		DescriptorPool				m_DescriptorPool		{ };
		std::vector<DescriptorSet>	m_vUniformDS			{ };
		std::vector<DescriptorSet>	m_vTextureDS			{ };

		// -- Command --
		CommandPool					m_CommandPool			{ };
		SyncManager					m_SyncManager			{ };

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void RecreateSwapChain();
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