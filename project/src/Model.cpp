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
#include "Debugger.h"


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
		aiProcess_OptimizeMeshes |
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
	CreateVertexBuffer(context, cmdPool);
	CreateIndexBuffer(context, cmdPool);

	// -- Build Image --
	uint32_t index = 0;
	for (Texture& tex : textures)
	{
		images.emplace_back();
		ImageBuilder builder{};
		builder
			.SetDebugName(std::ranges::find_if(pathToIdx, [&](auto& keyVal) { return keyVal.second == index; })->first.c_str())
			.SetWidth(tex.GetExtent().x)
			.SetHeight(tex.GetExtent().y)
			.SetFormat(VK_FORMAT_R8G8B8A8_SRGB)
			.SetTiling(VK_IMAGE_TILING_OPTIMAL)
			.SetUsageFlags(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.InitialData(tex.GetPixels(), 0, tex.GetExtent().x, tex.GetExtent().y, tex.GetMemorySize(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmdPool)
			.Build(context, images.back());
		images.back().CreateView(context, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);
		++index;
	}

	// -- Destroy Host Data --
	if (!keepHostData)
	{
		// - Free Textures --
		for (Texture& tex : textures)
			tex.FreePixels();

		// -- Clear Vectors --
		textures.clear();
		indices.clear();
		vertices.clear();
		pathToIdx.clear();
	}

}
void pom::Model::Destroy(const Context& context) const
{
	// -- Free Buffers --
	indexBuffer.Destroy(context);
	vertexBuffer.Destroy(context);

	// -- Free Textures --
	for (const Texture& tex : textures)
		tex.FreePixels();

	// -- Destroy Images --
	for (const Image& image : images)
		image.Destroy(context);
}

//--------------------------------------------------
//    Commands
//--------------------------------------------------
void pom::Model::Draw(CommandBuffer& cmdBuffer, const GraphicsPipelineLayout& pipelineLayout) const
{
	// -- Get Vulkan Command Buffer --
	const VkCommandBuffer& vCmdBuffer = cmdBuffer.GetHandle();

	// -- Bind Vertex Buffer --
	VkBuffer vertexBuffers[] = { vertexBuffer.GetHandle() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(vCmdBuffer, 0, 1, vertexBuffers, offsets);
	Debugger::InsertDebugLabel(cmdBuffer, "Bind Vertex Buffer", glm::vec4(0.6f, 1.f, 0.6f, 1.f));

	// -- Bind Index Buffer --
	vkCmdBindIndexBuffer(vCmdBuffer, indexBuffer.GetHandle(), 0, VK_INDEX_TYPE_UINT32);
	Debugger::InsertDebugLabel(cmdBuffer, "Bind Index Buffer", glm::vec4(1.f, 0.4f, 1.f, 1.f));

	// -- Draw Meshes --
	for (const Mesh& mesh : meshes)
	{
		// -- Bind Push Constants --
		MeshPushConstants pc { mesh.material.diffuseIdx };
		vkCmdPushConstants(
			vCmdBuffer,
			pipelineLayout.GetHandle(),
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(MeshPushConstants),
			&pc
		);
		Debugger::InsertDebugLabel(cmdBuffer, "Push Constants", glm::vec4(1.f, 0.6f, 0.f, 1.f));

		// -- Drawing Time! --
		vkCmdDrawIndexed(vCmdBuffer, mesh.indexCount, 1, mesh.indexOffset, mesh.vertexOffset, 0);
		Debugger::InsertDebugLabel(cmdBuffer, "Draw Mesh - " + mesh.name, glm::vec4(0.4f, 0.8f, 1.f, 1.f));
	}
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
	meshes.back().name = pMesh->mName.C_Str();

	// -- Process Vertices --
	meshes.back().vertexOffset = static_cast<uint32_t>(vertices.size());
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
									  pMesh->mNormals[vIdx].y,
									  pMesh->mNormals[vIdx].z);

		if (pMesh->mTextureCoords[0])
			vertex.texCoord = glm::vec2(pMesh->mTextureCoords[0][vIdx].x, pMesh->mTextureCoords[0][vIdx].y);
		else
			vertex.texCoord = glm::vec2(0.0f, 0.0f);

		vertex.color = glm::vec3(1.f, 1.f, 1.f);

		vertices.push_back(vertex);
	}

	// -- Process Indices --
	meshes.back().indexOffset = static_cast<uint32_t>(indices.size());
	for (uint32_t fIdx{}; fIdx < pMesh->mNumFaces; ++fIdx)
	{
		aiFace face = pMesh->mFaces[fIdx];
		for (uint32_t iIdx{}; iIdx < face.mNumIndices; ++iIdx)
			indices.push_back(face.mIndices[iIdx]);
	}
	meshes.back().indexCount = static_cast<uint32_t>(indices.size()) - meshes.back().indexOffset;

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
			meshes.back().material.diffuseIdx = idx;
			textures.emplace_back();
			textures.back().LoadFromFile(ss.str());
		}
		else
		{
			meshes.back().material.diffuseIdx = insertResult.first->second;
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

void pom::Model::CreateVertexBuffer(const Context& context, CommandPool& cmdPool)
{
	BufferAllocator alloc{};
	const uint32_t bufferSize = static_cast<uint32_t>(vertices.size()) * sizeof(Vertex);
	alloc
		.SetDebugName("Vertex Buffer")
		.SetSize(bufferSize)
		.SetUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
		.HostAccess(false)
		.InitialData(vertices.data(), 0, bufferSize, cmdPool)
		.Allocate(context, vertexBuffer);
}
void pom::Model::CreateIndexBuffer(const Context& context, CommandPool& cmdPool)
{
	BufferAllocator alloc{};
	const uint32_t bufferSize = static_cast<uint32_t>(indices.size()) * sizeof(uint32_t);
	alloc
		.SetDebugName("Index Buffer")
		.SetSize(bufferSize)
		.SetUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
		.HostAccess(false)
		.InitialData(indices.data(), 0, bufferSize, cmdPool)
		.Allocate(context, indexBuffer);
}
