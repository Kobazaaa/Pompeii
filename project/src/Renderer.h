#ifndef RENDERER_H
#define RENDERER_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>
#include "vma/vk_mem_alloc.h"

// -- Math Includes --
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtc/quaternion.hpp"

// -- Container Includes --
#include <vector>

// -- Helper Includes --
#include <optional>
#include <chrono>

// -- Custom Includes --
#include "CommandPool.h"
#include "DeletionQueue.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "Device.h"
#include "FrameBuffer.h"
#include "GraphicsPipeline.h"
#include "Image.h"
#include "Window.h"
#include "Instance.h"
#include "Model.h"
#include "PhysicalDevice.h"
#include "RenderPass.h"
#include "Sampler.h"
#include "SwapChain.h"


const std::string g_TEXTURE_PATH = "textures/viking_room.png";

constexpr int g_MAX_FRAMES_IN_FLIGHT = 2;

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

namespace pom
{
	class Window;

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
		void Initialize(Window* pWindow);
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
		Instance					m_Instance				{ };
		PhysicalDevice				m_PhysicalDevice		{ };
		Device						m_Device				{ };

		// -- SwapChain --
		SwapChain					m_SwapChain				{ };
		std::vector<FrameBuffer>	m_vFrameBuffers			{ };

		// -- Model --
		Sampler						m_TextureSampler		{ };
		Model						m_Model					{ };

		// -- Renderer --
		RenderPass					m_RenderPass			{ };
		GraphicsPipelineLayout		m_PipelineLayout		{ };
		GraphicsPipeline			m_GraphicsPipeline		{ };

		// -- Descriptors
		std::vector<Buffer>			m_vUniformBuffers		{ };
		DescriptorSetLayout			m_UniformDSL				{ };
		DescriptorSetLayout			m_TextureDSL				{ };
		DescriptorPool				m_DescriptorPool		{ };
		std::vector<DescriptorSet>	m_vUniformDS			{ };
		std::vector<DescriptorSet>	m_vTextureDS			{ };

		// -- Command --
		CommandPool					m_CommandPool			{ };
		SyncManager					m_SyncManager			{ };

		// -- VMA --
		VmaAllocator				m_Allocator				{ VK_NULL_HANDLE };

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void RecreateSwapChain();
		void LoadModels();
		void RecordCommandBuffer(CommandBuffer& commandBuffer, uint32_t imageIndex) const;
		void UpdateUniformBuffer(uint32_t currentImage) const;

		// -- Window --
		Window*				m_pWindow			{ };

		// -- Frame Counter --
		uint32_t			m_CurrentFrame		{ 0 };

		// -- Deletion Queues --
		DeletionQueue	m_DeletionQueue		{ };
		DeletionQueue	m_DeletionQueueSC	{ };

	};
}

#endif // RENDERER_H