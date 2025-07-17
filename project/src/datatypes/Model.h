#ifndef MODEL_DATA_TYPE_H
#define MODEL_DATA_TYPE_H

// -- Standard Library --
#include <vector>
#include <unordered_map>

// -- Pompeii Includes --
#include "Material.h"
#include "Shapes.h"

// -- Vulkan Includes
#include <vulkan/vulkan.h>

// -- Math Includes --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// -- Model Loading --
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

// -- Pompeii Includes --
#include "Buffer.h"
#include "Image.h"
using ModelHandle = uint32_t;

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Vertex	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
		glm::vec3 color;
		glm::vec2 texCoord;

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		static VkVertexInputBindingDescription GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
		bool operator==(const Vertex& other) const;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Mesh	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct Mesh
	{
		std::uint32_t vertexOffset;
		uint32_t indexOffset;
		uint32_t indexCount;

		Material material{};

		glm::mat4 matrix = glm::mat4(1);
		AABB aabb{};

		std::string name;
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Model	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct ModelCPU
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit ModelCPU(const std::string& path);
		virtual ~ModelCPU() = default;

		//--------------------------------------------------
		//    Data
		//--------------------------------------------------
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		std::vector<Texture> textures{};
		std::unordered_map<std::string, uint32_t> pathToIdx{};
		AABB aabb{};

		std::vector<Mesh> opaqueMeshes{};
		std::vector<Mesh> transparentMeshes{};

	private:
		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void ProcessNode(const aiNode* pNode, const aiScene* pScene, const glm::mat4& transform = glm::mat4(1.0f));
		void ProcessMesh(const aiMesh* pMesh, const aiScene* pScene, glm::mat4 transform);

		static glm::mat4 ConvertAssimpMatrix(const aiMatrix4x4& mat);
		inline static uint32_t globalTextureCounter{ 0 };
	};
	struct ModelGPU
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit ModelGPU() = default;
		virtual ~ModelGPU() = default;
		ModelGPU(const ModelGPU& other) = delete;
		ModelGPU(ModelGPU&& other) noexcept = delete;
		ModelGPU& operator=(const ModelGPU& other) = delete;
		ModelGPU& operator=(ModelGPU&& other) noexcept = delete;

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void Bind(CommandBuffer& cmdBuffer) const;
		void AllocateResources(const Context& context, const ModelCPU& modelCPU);
		void Destroy(const Context& context);

		//--------------------------------------------------
		//    Data
		//--------------------------------------------------
		Buffer vertexBuffer{};
		Buffer indexBuffer{};
		std::vector<Image> images{};

		std::vector<Mesh> opaqueMeshes{};
		std::vector<Mesh> transparentMeshes{};

	private:
		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void CreateVertexBuffer(const Context& context, const ModelCPU& modelCPU);
		void CreateIndexBuffer(const Context& context, const ModelCPU& modelCPU);
		void CreateImages(const Context& context, const ModelCPU& modelCPU);
	};
}

#endif // MODEL_DATA_TYPE_H