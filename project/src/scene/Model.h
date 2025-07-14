#ifndef MODEL_MESH_H
#define MODEL_MESH_H

// -- Standard Library --
#include <unordered_map>

// -- Vulkan Includes
#include <vulkan/vulkan.h>

// -- Defines --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS

// -- Math Includes --
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// -- Model Loading --
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

// -- Pompeii Includes --
#include "Material.h"
#include "Buffer.h"
#include "Image.h"

// -- Forward Declarations --
struct aiNode;
struct aiScene;
struct aiMesh;
namespace pompeii
{
	class CommandBuffer;
	class PipelineLayout;
	struct Context;
}

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
	//? ~~	  AABB	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct AABB
	{
		glm::vec3 min{  FLT_MAX };
		glm::vec3 max{ -FLT_MAX };

		void GrowToInclude(const glm::vec3& p)
		{
			min = glm::min(min, p);
			max = glm::max(max, p);
		}
		void GrowToInclude(const AABB& aabb)
		{
			min = glm::min(min, aabb.min);
			max = glm::max(max, aabb.max);
		}
};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Mesh	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct Mesh final
	{
		uint32_t vertexOffset;
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
	struct Model final
	{
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit Model() = default;
		~Model() = default;
		Model(const Model& other) = delete;
		Model(Model&& other) noexcept;
		Model& operator=(const Model& other) = delete;
		Model& operator=(Model&& other) noexcept;

		void LoadModel(const std::string& path);
		void AllocateResources(const Context& context, bool keepHostData = false);
		void Destroy(const Context& context);

		//--------------------------------------------------
		//    Commands
		//--------------------------------------------------
		void Bind(CommandBuffer& cmdBuffer) const;

		//--------------------------------------------------
		//    Data
		//--------------------------------------------------
		// -- CPU --
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		std::vector<Texture> textures{};
		std::unordered_map<std::string, uint32_t> pathToIdx{};
		AABB aabb{};

		// -- GPU --
		Buffer vertexBuffer;
		Buffer indexBuffer;
		std::vector<Image> images{};

		// -- Meshes --
		std::vector<Mesh> opaqueMeshes{};
		std::vector<Mesh> transparentMeshes{};

	private:
		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void ProcessNode(const aiNode* pNode, const aiScene* pScene, glm::mat4 transform = glm::mat4(1.0f));
		void ProcessMesh(const aiMesh* pMesh, const aiScene* pScene, glm::mat4 transform);

		static glm::mat4 ConvertAssimpMatrix(const aiMatrix4x4& mat);

		void CreateVertexBuffer(const Context& context);
		void CreateIndexBuffer(const Context& context);
		void CreateImages(const Context& context);

		inline static uint32_t globalTextureCounter{ 0 };
	};
}

#endif // MODEL_MESH_H