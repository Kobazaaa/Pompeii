#ifndef MESH_ASSET_H
#define MESH_ASSET_H

// -- Standard Library --
#include <span>
#include <string>
#include <vector>

// -- Pompeii Includes --
#include "Handle.h"
#include "Shapes.h"
#include "Buffer.h"

// -- Vulkan Includes
#include <vulkan/vulkan.h>

// -- Math Includes --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

		MaterialHandle material{};

		glm::mat4 matrix = glm::mat4(1);
		AABB aabb{};

		std::string name;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Create Info
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct MeshCreateInfo
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		std::vector<SubMesh> submeshes{};
		AABB aabb{};
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
		explicit Mesh() = default;
		~Mesh() = default;
		Mesh(const Mesh& other) = delete;
		Mesh(Mesh&& other) noexcept = default;
		Mesh& operator=(const Mesh& other) = delete;
		Mesh& operator=(Mesh&& other) noexcept = default;

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		static Mesh Create(const Context& context, const MeshCreateInfo& createInfo);
		void Bind(CommandBuffer& cmdBuffer) const;
		void Destroy(const Context& context);

		void UpdateVertices(const Context& context, std::span<const Vertex> vertices);
		void UpdateIndices(const Context& context, std::span<const uint32_t> indices);

		//--------------------------------------------------
		//    Data
		//--------------------------------------------------
		std::vector<SubMesh> vSubMeshes{};
		AABB aabb{};

		Buffer vertexBuffer{};
		Buffer indexBuffer{};

	private:
		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		static void CreateVertexBuffer(const Context& context, std::span<const Vertex> vertices, Buffer& outBuffer);
		static void CreateIndexBuffer(const Context& context, std::span<const uint32_t> indices, Buffer& outBuffer);
	};
}

#endif // MESH_ASSET_H
