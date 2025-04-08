// -- Standard Library --
#include <iostream>
#include <sstream>

// -- Model Loading --
#include <assimp/postprocess.h>

// -- Pompeii Includes --
#include "Model.h"
#include "GraphicsPipeline.h"
#include "Context.h"
#include "CommandBuffer.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Vertex	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Helpers
//--------------------------------------------------

VkVertexInputBindingDescription pom::Vertex::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}
std::vector<VkVertexInputAttributeDescription> pom::Vertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);
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
	attributeDescriptions[2].offset = offsetof(Vertex, color);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

	return attributeDescriptions;
}

bool pom::Vertex::operator==(const Vertex& other) const
{
	return position == other.position &&
		color == other.color &&
		texCoord == other.texCoord;
}


//--------------------------------------------------
//    Commands
//--------------------------------------------------
void pom::Mesh::Destroy(const Context& context) const
{
	indexBuffer.Destroy(context);
	vertexBuffer.Destroy(context);
}
void pom::Mesh::Draw(CommandBuffer& cmdBuffer, const GraphicsPipelineLayout& pipelineLayout) const
{
	// -- Get Vulkan Command Buffer --
	const VkCommandBuffer& vCmdBuffer = cmdBuffer.GetHandle();

	// -- Bind Push Constants --
	vkCmdPushConstants(
		vCmdBuffer,
		pipelineLayout.GetHandle(),
		VK_SHADER_STAGE_FRAGMENT_BIT,
		0,
		sizeof(MeshPushConstants),
		&pc
	);

	// -- Bind Vertex Buffer --
	VkBuffer vertexBuffers[] = { vertexBuffer.GetHandle() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(vCmdBuffer, 0, 1, vertexBuffers, offsets);

	// -- Bind Index Buffer --
	vkCmdBindIndexBuffer(vCmdBuffer, indexBuffer.GetHandle(), 0, VK_INDEX_TYPE_UINT32);

	// -- Drawing Time! --
	vkCmdDrawIndexed(vCmdBuffer, indexCount, 1, 0, 0, 0);
}


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Model	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::Model::LoadModel(const std::string& path)
{
	Assimp::Importer importer;

	const aiScene* pScene =
		importer.ReadFile(path,
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		//aiProcess_FlipWindingOrder |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
	{
		std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << "\n";
		return;
	}

	ProcessNode(pScene->mRootNode, pScene, ConvertAssimpMatrix(pScene->mRootNode->mTransformation));
}
void pom::Model::AllocateResources(const Context& context, CommandPool& cmdPool, bool keepHostData)
{
	CreateVertexBuffers(context, cmdPool);
	CreateIndexBuffers(context, cmdPool);

	// -- Build Image --
	for (Texture& tex : textures)
	{
		images.emplace_back();
		ImageBuilder builder{};
		builder.SetWidth(tex.GetExtent().x)
			.SetHeight(tex.GetExtent().y)
			.SetFormat(VK_FORMAT_R8G8B8A8_SRGB)
			.SetTiling(VK_IMAGE_TILING_OPTIMAL)
			.SetUsageFlags(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.InitialData(tex.GetPixels(), 0, tex.GetExtent().x, tex.GetExtent().y, tex.GetMemorySize(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.Build(context, cmdPool, images.back());
		images.back().GenerateImageView(context, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);
	}

	// -- Destroy Host Data --
	if (!keepHostData)
	{
		for (Texture& tex : textures)
			tex.FreePixels();
		textures.clear();

		for (Mesh& mesh : meshes)
		{
			mesh.indices.clear();
			mesh.vertices.clear();
		}

		pathToIdx.clear();
	}

}
void pom::Model::Destroy(const Context& context) const
{
	for (const Texture& tex : textures)
		tex.FreePixels();
	for (const Image& image : images)
		image.Destroy(context);

	for (int index{ static_cast<int>(meshes.size()) - 1 }; index >= 0; --index)
		meshes[index].Destroy(context);
}

//--------------------------------------------------
//    Commands
//--------------------------------------------------
void pom::Model::Draw(CommandBuffer& cmdBuffer, const GraphicsPipelineLayout& pipelineLayout) const
{
	for (const Mesh& mesh : meshes)
		mesh.Draw(cmdBuffer, pipelineLayout);
}


//--------------------------------------------------
//    Helpers
//--------------------------------------------------
void pom::Model::ProcessNode(const aiNode* pNode, const aiScene* pScene, const glm::mat4& parentTransform)
{
	glm::mat4 nodeTransform = ConvertAssimpMatrix(pNode->mTransformation);
	glm::mat4 globalTransform = parentTransform * nodeTransform;

	for (uint32_t index{}; index < pNode->mNumMeshes; ++index)
	{
		aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[index]];
		ProcessMesh(pMesh, pScene, globalTransform);
	}

	for (uint32_t cIdx{}; cIdx < pNode->mNumChildren; ++cIdx)
		ProcessNode(pNode->mChildren[cIdx], pScene, globalTransform);
}
void pom::Model::ProcessMesh(const aiMesh* pMesh, const aiScene* pScene, const glm::mat4& transform)
{
	meshes.emplace_back();

	// -- Process Vertices --
	for (uint32_t vIdx{}; vIdx < pMesh->mNumVertices; ++vIdx)
	{
		Vertex vertex;

		vertex.position = glm::vec3(pMesh->mVertices[vIdx].x,
									pMesh->mVertices[vIdx].y,
									pMesh->mVertices[vIdx].z);
		glm::vec4 pos = transform * glm::vec4(vertex.position, 1);
		vertex.position = glm::vec3(pos);

		if (pMesh->HasNormals())
			vertex.normal = glm::vec3(pMesh->mNormals[vIdx].x,
									  pMesh->mNormals[vIdx].z,
									  pMesh->mNormals[vIdx].y);

		if (pMesh->mTextureCoords[0])
			vertex.texCoord = glm::vec2(pMesh->mTextureCoords[0][vIdx].x, pMesh->mTextureCoords[0][vIdx].y);
		else
			vertex.texCoord = glm::vec2(0.0f, 0.0f);

		vertex.color = glm::vec3(1.f, 1.f, 1.f);

		meshes.back().vertices.push_back(vertex);
	}
	meshes.back().vertexCount = static_cast<uint32_t>(meshes.back().vertices.size());

	// -- Process Indices --
	for (uint32_t fIdx{}; fIdx < pMesh->mNumFaces; ++fIdx)
	{
		aiFace face = pMesh->mFaces[fIdx];
		for (uint32_t iIdx{}; iIdx < face.mNumIndices; ++iIdx)
			meshes.back().indices.push_back(face.mIndices[iIdx]);
	}
	meshes.back().indexCount = static_cast<uint32_t>(meshes.back().indices.size());

	// -- Process Materials --
	const aiMaterial* material = pScene->mMaterials[pMesh->mMaterialIndex];

	// -- Diffuse --
	uint32_t count = material->GetTextureCount(aiTextureType_DIFFUSE);
	for (uint32_t mIdx{}; mIdx < count; ++mIdx)
	{
		aiString texturePath;
		material->GetTexture(aiTextureType_DIFFUSE, mIdx, &texturePath);
		std::stringstream ss;
		ss << "textures/" << texturePath.C_Str();

		uint32_t idx = static_cast<uint32_t>(textures.size());
		auto insertResult = pathToIdx.insert({ss.str(), idx});
		if (insertResult.second)
		{
			meshes.back().material.textureIdx = idx;
			meshes.back().pc = { idx };
			textures.emplace_back();
			textures.back().LoadFromFile(ss.str());
		}
		else
		{
			meshes.back().material.textureIdx = insertResult.first->second;
			meshes.back().pc = { insertResult.first->second };
		}
	}
}

glm::mat4 pom::Model::ConvertAssimpMatrix(const aiMatrix4x4& mat)
{
	return glm::mat4(
		mat.a1, mat.b1, mat.c1, mat.d1,
		mat.a2, mat.b2, mat.c2, mat.d2,
		mat.a3, mat.b3, mat.c3, mat.d3,
		mat.a4, mat.b4, mat.c4, mat.d4
	);
}

void pom::Model::CreateVertexBuffers(const Context& context, CommandPool& cmdPool)
{
	for (Mesh& mesh : meshes)
	{
		BufferAllocator alloc{};

		const uint32_t bufferSize = static_cast<uint32_t>(mesh.vertices.size()) * sizeof(mesh.vertices[0]);
		alloc
			.SetSize(bufferSize)
			.SetUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
			.HostAccess(false)
			.InitialData(mesh.vertices.data(), 0, bufferSize)
			.Allocate(context, cmdPool, mesh.vertexBuffer);
	}
}
void pom::Model::CreateIndexBuffers(const Context& context, CommandPool& cmdPool)
{
	for (Mesh& mesh : meshes)
	{
		BufferAllocator alloc{};

		const uint32_t bufferSize = static_cast<uint32_t>(mesh.indices.size()) * sizeof(mesh.indices[0]);
		alloc
			.SetSize(bufferSize)
			.SetUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
			.HostAccess(false)
			.InitialData(mesh.indices.data(), 0, bufferSize)
			.Allocate(context, cmdPool, mesh.indexBuffer);
	}
}
