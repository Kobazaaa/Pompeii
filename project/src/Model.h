#ifndef MESH_H
#define MESH_H

// -- Math Includes --
#include <glm/glm.hpp>

// -- Custom Includes --
#include "DescriptorPool.h"

struct aiNode;
struct aiScene;
struct aiMesh;
namespace pom
{
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
	struct Mesh
	{
		std::vector<Vertex> vertices;
		Buffer vertexBuffer;

		std::vector<uint32_t> indices;
		Buffer indexBuffer;

		// Material (soon TM)
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
		void AllocateBuffers(const Device& device, const VmaAllocator& allocator, CommandPool& cmdPool);
		void DestroyBuffers(const Device& device, const VmaAllocator& allocator) const;

		std::vector<Mesh> meshes;

	private:
		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void ProcessNode(const aiNode* pNode, const aiScene* pScene);
		void ProcessMesh(const aiMesh* pMesh, const aiScene* pScene);

		void CreateVertexBuffers(const Device& device, VmaAllocator allocator, CommandPool& cmdPool);
		void CreateIndexBuffers(const Device& device, const VmaAllocator allocator, CommandPool& cmdPool);
	};
}

#endif // MESH_H