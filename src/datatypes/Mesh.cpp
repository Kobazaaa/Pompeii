// -- Pompeii Includes --
#include "Mesh.h"
#include "CommandBuffer.h"
#include "RenderDebugger.h"

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Vertex
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
VkVertexInputBindingDescription pompeii::Vertex::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}
std::vector<VkVertexInputAttributeDescription> pompeii::Vertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(6);
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, normal);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, tangent);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(Vertex, bitangent);

	attributeDescriptions[4].binding = 0;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[4].offset = offsetof(Vertex, color);

	attributeDescriptions[5].binding = 0;
	attributeDescriptions[5].location = 5;
	attributeDescriptions[5].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[5].offset = offsetof(Vertex, texCoord);

	return attributeDescriptions;
}
bool pompeii::Vertex::operator==(const Vertex& other) const
{
	return position == other.position &&
		normal == other.normal &&
		tangent == other.tangent &&
		bitangent == other.bitangent &&
		color == other.color &&
		texCoord == other.texCoord;
}


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Mesh
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Helpers
//--------------------------------------------------
pompeii::Mesh pompeii::Mesh::Create(const Context& context, const MeshCreateInfo& createInfo)
{
	Mesh mesh{};
	mesh.vSubMeshes = createInfo.submeshes;
	mesh.aabb = createInfo.aabb;

	CreateVertexBuffer(context, createInfo.vertices, mesh.vertexBuffer);
	CreateIndexBuffer(context, createInfo.indices, mesh.indexBuffer);

	return mesh;
}
void pompeii::Mesh::Bind(CommandBuffer& cmdBuffer) const
{
	// -- Get Vulkan Command Buffer --
	const VkCommandBuffer& vCmdBuffer = cmdBuffer.GetHandle();

	// -- Bind Vertex Buffer --
	VkBuffer vertexBuffers[] = { vertexBuffer.GetHandle() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(vCmdBuffer, 0, 1, vertexBuffers, offsets);
	RenderDebugger::InsertDebugLabel(cmdBuffer, "Bind Vertex Buffer", glm::vec4(0.6f, 1.f, 0.6f, 1.f));

	// -- Bind Index Buffer --
	vkCmdBindIndexBuffer(vCmdBuffer, indexBuffer.GetHandle(), 0, VK_INDEX_TYPE_UINT32);
	RenderDebugger::InsertDebugLabel(cmdBuffer, "Bind Index Buffer", glm::vec4(1.f, 0.4f, 1.f, 1.f));
}
void pompeii::Mesh::Destroy(const Context& context)
{
	indexBuffer.Destroy(context);
	vertexBuffer.Destroy(context);
}

void pompeii::Mesh::UpdateVertices(const Context& context, std::span<const Vertex> vertices)
{
	vertexBuffer.Destroy(context);
	CreateVertexBuffer(context, vertices, vertexBuffer);
}
void pompeii::Mesh::UpdateIndices(const Context& context, std::span<const uint32_t> indices)
{
	indexBuffer.Destroy(context);
	CreateIndexBuffer(context, indices, indexBuffer);
}

void pompeii::Mesh::CreateVertexBuffer(const Context& context, std::span<const Vertex> vertices, Buffer& outBuffer)
{
	BufferAllocator alloc{};
	const uint32_t bufferSize = static_cast<uint32_t>(vertices.size() * sizeof(Vertex));
	alloc
		.SetDebugName("Vertex Buffer")
		.SetSize(bufferSize)
		.SetUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
		.HostAccess(false)
		.AddInitialData(vertices.data(), 0, bufferSize)
		.Allocate(context, outBuffer);
}
void pompeii::Mesh::CreateIndexBuffer(const Context& context, std::span<const uint32_t> indices, Buffer& outBuffer)
{
	BufferAllocator alloc{};
	const uint32_t bufferSize = static_cast<uint32_t>(indices.size() * sizeof(uint32_t));
	alloc
		.SetDebugName("Index Buffer")
		.SetSize(bufferSize)
		.SetUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
		.HostAccess(false)
		.AddInitialData(indices.data(), 0, bufferSize)
		.Allocate(context, outBuffer);
}
