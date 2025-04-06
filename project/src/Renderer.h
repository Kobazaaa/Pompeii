#ifndef RENDERER_H
#define RENDERER_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>
#include "vma/vk_mem_alloc.h"

// -- Math Includes --
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtc/quaternion.hpp"
#include <glm/gtx/hash.hpp>

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
#include "PhysicalDevice.h"
#include "RenderPass.h"
#include "Sampler.h"
#include "SwapChain.h"


const std::string g_MODEL_PATH = "models/viking_room.obj";
const std::string g_TEXTURE_PATH = "textures/viking_room.png";

constexpr int g_MAX_FRAMES_IN_FLIGHT = 2;

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}
	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const
	{
		return position == other.position &&
			color == other.color &&
			texCoord == other.texCoord;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

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
		Image						m_TextureImage			{ };
		Sampler						m_TextureSampler		{ };
		std::vector<Vertex>			m_vVertices;
		std::vector<uint32_t>		m_vIndices;
		Buffer						m_VertexBuffer			{ };
		Buffer						m_IndexBuffer			{ };

		// -- Renderer --
		RenderPass					m_RenderPass			{ };
		GraphicsPipelineLayout		m_PipelineLayout		{ };
		GraphicsPipeline			m_GraphicsPipeline		{ };

		// -- Descriptors
		std::vector<Buffer>			m_vUniformBuffers		{ };
		DescriptorSetLayout			m_DescriptorSetLayout	{ };
		DescriptorPool				m_DescriptorPool		{ };
		std::vector<DescriptorSet>	m_vDescriptorSets		{ };

		// -- Command --
		CommandPool					m_CommandPool			{ };
		SyncManager					m_SyncManager			{ };

		// -- VMA --
		VmaAllocator				m_Allocator				{ VK_NULL_HANDLE };

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void RecreateSwapChain();
		void CreateTextureImage();
		void LoadModel();
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