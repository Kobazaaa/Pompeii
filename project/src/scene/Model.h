#ifndef MODEL_MESH_H
#define MODEL_MESH_H

// -- Standard Library --
#include <unordered_map>

// -- Vulkan Includes
#include <vulkan/vulkan.h>

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
namespace pom
{
	class CommandPool;
	class CommandBuffer;
	class GraphicsPipelineLayout;
	struct Context;
}


namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Model Push Constants	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct MeshPushConstants
	{
		uint32_t diffuseIdx;
		uint32_t opacityIdx;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Vertex	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
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
	struct Mesh final
	{
		uint32_t vertexOffset;
		uint32_t indexOffset;
		uint32_t indexCount;

		Material material;

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
		void LoadModel(const std::string& path);
		void AllocateResources(const Context& context, CommandPool& cmdPool, bool keepHostData = false);
		void Destroy(const Context& context) const;

		//--------------------------------------------------
		//    Commands
		//--------------------------------------------------
		void Bind(CommandBuffer& cmdBuffer) const;
		void DrawOpaque(CommandBuffer& cmdBuffer, const GraphicsPipelineLayout& pipelineLayout) const;
		void DrawTransparent(CommandBuffer& cmdBuffer, const GraphicsPipelineLayout& pipelineLayout) const;


		//--------------------------------------------------
		//    Data
		//--------------------------------------------------
		// -- CPU --
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<Texture> textures;
		std::unordered_map<std::string, uint32_t> pathToIdx;

		// -- GPU --
		Buffer vertexBuffer;
		Buffer indexBuffer;
		std::vector<Image> images;

		// -- Meshes --
		std::vector<Mesh> opaqueMeshes;
		std::vector<Mesh> transparentMeshes;

	private:
		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void ProcessNode(const aiNode* pNode, const aiScene* pScene, const glm::mat4& parentTransform);
		void ProcessMesh(const aiMesh* pMesh, const aiScene* pScene, const glm::mat4& transform);

		static glm::mat4 ConvertAssimpMatrix(const aiMatrix4x4& mat);

		void CreateVertexBuffer(const Context& context, CommandPool& cmdPool);
		void CreateIndexBuffer(const Context& context, CommandPool& cmdPool);
	};
}

#endif // MODEL_MESH_H