#ifndef MODEL_MESH_H
#define MODEL_MESH_H

// -- Containers --
#include <unordered_map>

// -- Math Includes --
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// -- Model Loading --
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

// -- Custom Includes --
#include "CommandBuffer.h"
#include "GraphicsPipeline.h"
#include "Material.h"
#include "DescriptorPool.h"

struct aiNode;
struct aiScene;
struct aiMesh;
namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Model Push Constants	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct MeshPushConstants
	{
		uint32_t diffuseIdx;
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
		//--------------------------------------------------
		//    Commands
		//--------------------------------------------------
		void Destroy(const Device& device, const VmaAllocator& allocator) const;
		void Draw(CommandBuffer& cmdBuffer, const GraphicsPipelineLayout& pipelineLayout) const;

		//--------------------------------------------------
		//    Data
		//--------------------------------------------------
		std::vector<Vertex> vertices;
		Buffer vertexBuffer;

		std::vector<uint32_t> indices;
		Buffer indexBuffer;

		MeshPushConstants pc;
		Material material;
		uint32_t indexCount;
		uint32_t vertexCount;
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
		void AllocateResources(const Device& device, const VmaAllocator& allocator, CommandPool& cmdPool, bool keepHostData = false);
		void Destroy(const Device& device, const VmaAllocator& allocator) const;

		//--------------------------------------------------
		//    Commands
		//--------------------------------------------------
		void Draw(CommandBuffer& cmdBuffer, const GraphicsPipelineLayout& pipelineLayout) const;


		//--------------------------------------------------
		//    Data
		//--------------------------------------------------
		std::vector<Mesh> meshes;
		std::vector<Texture> textures;
		std::vector<Image> images;
		std::unordered_map<std::string, uint32_t> pathToIdx;

	private:
		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void ProcessNode(const aiNode* pNode, const aiScene* pScene, const glm::mat4& parentTransform);
		void ProcessMesh(const aiMesh* pMesh, const aiScene* pScene, const glm::mat4& transform);

		static glm::mat4 ConvertAssimpMatrix(const aiMatrix4x4& mat);

		void CreateVertexBuffers(const Device& device, VmaAllocator allocator, CommandPool& cmdPool);
		void CreateIndexBuffers(const Device& device, const VmaAllocator allocator, CommandPool& cmdPool);
	};
}

#endif // MODEL_MESH_H