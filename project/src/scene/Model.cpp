// -- Standard Library --
#include <iostream>
#include <sstream>

// -- Model Loading --
#include <assimp/postprocess.h>

// -- Pompeii Includes --
#include "Model.h"
#include "Pipeline.h"
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

bool pom::Vertex::operator==(const Vertex& other) const
{
	return position == other.position &&
		normal == other.normal &&
		tangent == other.tangent &&
		bitangent == other.bitangent &&
		color == other.color &&
		texCoord == other.texCoord;
}


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Model	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::Model::Model(Model&& other) noexcept
{
	vertices = std::move(other.vertices);
	other.vertices.clear();
	indices = std::move(other.indices);
	other.indices.clear();
	textures = std::move(other.textures);
	other.textures.clear();
	pathToIdx = std::move(other.pathToIdx);
	other.pathToIdx.clear();
	vertexBuffer = std::move(other.vertexBuffer);
	indexBuffer = std::move(other.indexBuffer);
	images = std::move(other.images);
	other.images.clear();
	opaqueMeshes = std::move(other.opaqueMeshes);
	other.opaqueMeshes.clear();
	transparentMeshes = std::move(other.transparentMeshes);
	other.transparentMeshes.clear();
}
pom::Model& pom::Model::operator=(Model&& other) noexcept
{
	if (this == &other)
		return *this;
	vertices = std::move(other.vertices);
	other.vertices.clear();
	indices = std::move(other.indices);
	other.indices.clear();
	textures = std::move(other.textures);
	other.textures.clear();
	pathToIdx = std::move(other.pathToIdx);
	other.pathToIdx.clear();
	vertexBuffer = std::move(other.vertexBuffer);
	indexBuffer = std::move(other.indexBuffer);
	images = std::move(other.images);
	other.images.clear();
	opaqueMeshes = std::move(other.opaqueMeshes);
	other.opaqueMeshes.clear();
	transparentMeshes = std::move(other.transparentMeshes);
	other.transparentMeshes.clear();
	return *this;
}

void pom::Model::LoadModel(const std::string& path)
{
	Assimp::Importer importer;
	const aiScene* pScene =
		importer.ReadFile(path,
		aiProcess_Triangulate |
		aiProcess_OptimizeMeshes |
		aiProcess_CalcTangentSpace |
		aiProcess_ConvertToLeftHanded |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
	{
		std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << "\n";
		return;
	}

	ProcessNode(pScene->mRootNode, pScene);
}
void pom::Model::AllocateResources(const Context& context, bool keepHostData)
{
	// -- Create Buffers --
	CreateVertexBuffer(context);
	CreateIndexBuffer(context);

	// -- Build Image --
	CreateImages(context);

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
void pom::Model::Destroy(const Context& context)
{
	// -- Flush --
	for (Image& image : images) 
		image.Destroy(context);
	indexBuffer.Destroy(context);
	vertexBuffer.Destroy(context);

	// -- Free Textures --
	for (Texture& tex : textures)
		tex.FreePixels();
}


//--------------------------------------------------
//    Commands
//--------------------------------------------------
void pom::Model::Bind(CommandBuffer& cmdBuffer) const
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
}

//--------------------------------------------------
//    Helpers
//--------------------------------------------------
void pom::Model::ProcessNode(const aiNode* pNode, const aiScene* pScene)
{
	for (uint32_t index{}; index < pNode->mNumMeshes; ++index)
	{
		aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[index]];
		ProcessMesh(pMesh, pScene);
	}

	for (uint32_t cIdx{}; cIdx < pNode->mNumChildren; ++cIdx)
		ProcessNode(pNode->mChildren[cIdx], pScene);
}
void pom::Model::ProcessMesh(const aiMesh* pMesh, const aiScene* pScene)
{
	opaqueMeshes.push_back(Mesh());
	opaqueMeshes.back().name = pMesh->mName.C_Str();
	opaqueMeshes.back().matrix = ConvertAssimpMatrix(pScene->mRootNode->mTransformation);

	// -- Process Vertices --
	opaqueMeshes.back().vertexOffset = static_cast<uint32_t>(vertices.size());
	for (uint32_t vIdx{}; vIdx < pMesh->mNumVertices; ++vIdx)
	{
		Vertex vertex{};

		vertex.position = glm::vec3(pMesh->mVertices[vIdx].x,
									pMesh->mVertices[vIdx].y,
									pMesh->mVertices[vIdx].z);

		if (pMesh->HasNormals())
			vertex.normal = glm::vec3(pMesh->mNormals[vIdx].x,
									  pMesh->mNormals[vIdx].y,
									  pMesh->mNormals[vIdx].z);

		if (pMesh->mTextureCoords[0])
			vertex.texCoord = glm::vec2(pMesh->mTextureCoords[0][vIdx].x, pMesh->mTextureCoords[0][vIdx].y);
		else
			vertex.texCoord = glm::vec2(0.0f, 0.0f);

		vertex.color = glm::vec3(1.f, 1.f, 1.f);

		if (pMesh->HasTangentsAndBitangents())
		{
			vertex.tangent = glm::vec3(pMesh->mTangents[vIdx].x,
									   pMesh->mTangents[vIdx].y,
									   pMesh->mTangents[vIdx].z);
			vertex.bitangent = glm::vec3(pMesh->mBitangents[vIdx].x,
										 pMesh->mBitangents[vIdx].y,
										 pMesh->mBitangents[vIdx].z);
		}
		vertices.push_back(vertex);
	}

	// -- Process Indices --
	opaqueMeshes.back().indexOffset = static_cast<uint32_t>(indices.size());
	for (uint32_t fIdx{}; fIdx < pMesh->mNumFaces; ++fIdx)
	{
		aiFace face = pMesh->mFaces[fIdx];
		for (uint32_t iIdx{}; iIdx < face.mNumIndices; ++iIdx)
			indices.push_back(face.mIndices[iIdx]);
	}
	opaqueMeshes.back().indexCount = static_cast<uint32_t>(indices.size()) - opaqueMeshes.back().indexOffset;

	// -- Process Materials --
	const aiMaterial* material = pScene->mMaterials[pMesh->mMaterialIndex];
	auto LoadMatTexture = [&](aiTextureType type, uint32_t& targetIdx, VkFormat format)
		{
			uint32_t count = material->GetTextureCount(type);
			for (uint32_t mIdx{}; mIdx < count; ++mIdx)
			{
				aiString texturePath;
				material->GetTexture(type, mIdx, &texturePath);
				std::string fullPath = "textures/" + std::string(texturePath.C_Str());

				uint32_t idx = Texture::GetStaticIndex();
				auto [it, succeeded] = pathToIdx.insert({ fullPath, idx });
				targetIdx = it->second;

				if (succeeded)
					textures.emplace_back(fullPath, format);
			}
		};

	// -- Diffuse --
	auto& mat = opaqueMeshes.back().material;
	LoadMatTexture(aiTextureType_DIFFUSE,			mat.albedoIdx,		VK_FORMAT_R8G8B8A8_SRGB);

	LoadMatTexture(aiTextureType_SPECULAR,			mat.specularIdx,		VK_FORMAT_R8G8B8A8_UNORM);
	LoadMatTexture(aiTextureType_SHININESS,			mat.shininessIdx,	VK_FORMAT_R8G8B8A8_UNORM);

	LoadMatTexture(aiTextureType_HEIGHT,			mat.heightIdx,		VK_FORMAT_R8G8B8A8_UNORM);
	LoadMatTexture(aiTextureType_NORMALS,			mat.normalIdx,		VK_FORMAT_R8G8B8A8_UNORM);

	LoadMatTexture(aiTextureType_DIFFUSE_ROUGHNESS, mat.roughnessIdx,	VK_FORMAT_R8G8B8A8_UNORM);
	LoadMatTexture(aiTextureType_METALNESS,			mat.metalnessIdx,	VK_FORMAT_R8G8B8A8_UNORM);

	// Transparency Maps Separate --
	if (material->GetTextureCount(aiTextureType_OPACITY) > 0)
	{
		transparentMeshes.push_back(opaqueMeshes.back());
		opaqueMeshes.pop_back();

		auto& transMat = transparentMeshes.back().material;
		LoadMatTexture(aiTextureType_OPACITY, transMat.opacityIdx, VK_FORMAT_R8G8B8A8_UNORM);
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

void pom::Model::CreateVertexBuffer(const Context& context)
{
	BufferAllocator alloc{};
	const uint32_t bufferSize = static_cast<uint32_t>(vertices.size()) * sizeof(Vertex);
	alloc
		.SetDebugName("Vertex Buffer")
		.SetSize(bufferSize)
		.SetUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
		.HostAccess(false)
		.AddInitialData(vertices.data(), 0, bufferSize)
		.Allocate(context, vertexBuffer);
}
void pom::Model::CreateIndexBuffer(const Context& context)
{
	BufferAllocator alloc{};
	const uint32_t bufferSize = static_cast<uint32_t>(indices.size()) * sizeof(uint32_t);
	alloc
		.SetDebugName("Index Buffer")
		.SetSize(bufferSize)
		.SetUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
		.HostAccess(false)
		.AddInitialData(indices.data(), 0, bufferSize)
		.Allocate(context, indexBuffer);
}
void pom::Model::CreateImages(const Context& context)
{
	for (Texture& tex : textures)
	{
		images.emplace_back();

		uint32_t texW = tex.GetExtent().x;
		uint32_t texH = tex.GetExtent().y;
		uint32_t maxMipsLevels = 1;
		// only generate mipmaps for big enough textures
		if (texW >= 256 || texH >= 256)
			maxMipsLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texW, texH)))) + 1;
		uint32_t mipLevels = maxMipsLevels;

		ImageBuilder builder{};
		builder
			.SetDebugName(std::ranges::find_if(pathToIdx, [&](auto& keyVal) { return keyVal.second == tex.GetLocalIndex(); })->first.c_str())
			.SetWidth(texW)
			.SetHeight(texH)
			.SetFormat(tex.GetFormat())
			.SetTiling(VK_IMAGE_TILING_OPTIMAL)
			.SetMipLevels(mipLevels)
			.SetUsageFlags(VK_IMAGE_USAGE_SAMPLED_BIT)
			.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.InitialData(tex.GetPixels(), 0, texW, texH, tex.GetMemorySize(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.Build(context, images.back());
		images.back().CreateView(context, tex.GetFormat(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, mipLevels, 0, 1);
	}
}
