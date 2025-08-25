#ifndef MESH_ASSET_H
#define MESH_ASSET_H

// -- Standard Library --
#include <vector>
#include <unordered_map>

// -- Pompeii Includes --
#include "Material.h"
#include "Shapes.h"
#include "Buffer.h"
#include "Image.h"

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
	struct SubMesh
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
	//? ~~	  Mesh	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct Mesh
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit Mesh(const std::string& path);
		virtual ~Mesh() = default;
		Mesh(const Mesh& other) = delete;
		Mesh(Mesh&& other) noexcept = delete;
		Mesh& operator=(const Mesh& other) = delete;
		Mesh& operator=(Mesh&& other) noexcept = delete;

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void Bind(CommandBuffer& cmdBuffer) const;
		void AllocateResources(const Context& context);
		void Destroy(const Context& context);

		//--------------------------------------------------
		//    CPU Data
		//--------------------------------------------------
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		std::vector<Texture> textures{};
		std::unordered_map<std::string, uint32_t> pathToIdx{};
		std::vector<SubMesh> vSubMeshes{};
		AABB aabb{};

		//--------------------------------------------------
		//    GPU Data
		//--------------------------------------------------
		Buffer vertexBuffer{};
		Buffer indexBuffer{};
		std::vector<Image> images{};

	private:
		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void ProcessNode(const aiNode* pNode, const aiScene* pScene, const glm::mat4& transform = glm::mat4(1.0f));
		void ProcessMesh(const aiMesh* pMesh, const aiScene* pScene, glm::mat4 transform);

		void CreateVertexBuffer(const Context& context);
		void CreateIndexBuffer(const Context& context);
		void CreateImages(const Context& context);

		static glm::mat4 ConvertAssimpMatrix(const aiMatrix4x4& mat);
	};
}

#endif // MESH_ASSET_H