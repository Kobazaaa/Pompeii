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
void pom::Model::LoadModel(const std::string& path)
{
	Assimp::Importer importer;
	const aiScene* pScene =
		importer.ReadFile(path,
		aiProcess_Triangulate |
		aiProcess_OptimizeMeshes |
		aiProcess_CalcTangentSpace |
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
	// -- Create Buffers --
	CreateVertexBuffer(context, cmdPool);
	CreateIndexBuffer(context, cmdPool);

	// -- Build Image --
	CreateImages(context, cmdPool);

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
void pom::Model::Destroy()
{
	// -- Flush --
	deletionQueue.Flush();

	// -- Free Textures --
	for (const Texture& tex : textures)
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

void pom::Model::DrawOpaque(CommandBuffer& cmdBuffer, const GraphicsPipelineLayout& pipelineLayout) const
{
	// -- Get Vulkan Command Buffer --
	const VkCommandBuffer& vCmdBuffer = cmdBuffer.GetHandle();

	// -- Draw Meshes --
	for (const Mesh& mesh : opaqueMeshes)
	{
		// -- Bind Push Constants --
		MeshPushConstants pc
		{
			.diffuseIdx = mesh.material.diffuseIdx,
			.opacityIdx = mesh.material.opacityIdx,
			.specularIdx = mesh.material.specularIdx,
			.shininessIdx = mesh.material.shininessIdx,
			.heightIdx = mesh.material.heightIdx,
			.exp = mesh.material.exp
		};
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
		Debugger::InsertDebugLabel(cmdBuffer, "Draw Opaque Mesh - " + mesh.name, glm::vec4(0.4f, 0.8f, 1.f, 1.f));
	}
}
void pom::Model::DrawTransparent(CommandBuffer& cmdBuffer, const GraphicsPipelineLayout& pipelineLayout) const
{
	// -- Get Vulkan Command Buffer --
	const VkCommandBuffer& vCmdBuffer = cmdBuffer.GetHandle();

	// -- Draw Meshes --
	for (const Mesh& mesh : transparentMeshes)
	{
		// -- Bind Push Constants --
		MeshPushConstants pc
		{
			.diffuseIdx = mesh.material.diffuseIdx,
			.opacityIdx = mesh.material.opacityIdx,
			.specularIdx = mesh.material.specularIdx,
			.shininessIdx = mesh.material.shininessIdx,
			.heightIdx = mesh.material.heightIdx,
			.exp = mesh.material.exp
		};
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
		Debugger::InsertDebugLabel(cmdBuffer, "Draw Transparent Mesh - " + mesh.name, glm::vec4(0.4f, 0.8f, 1.f, 1.f));
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
	opaqueMeshes.emplace_back();
	opaqueMeshes.back().name = pMesh->mName.C_Str();

	// -- Process Vertices --
	opaqueMeshes.back().vertexOffset = static_cast<uint32_t>(vertices.size());
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
			opaqueMeshes.back().material.diffuseIdx = idx;
			textures.emplace_back();
			textures.back().LoadFromFile(ss.str());
		}
		else
		{
			opaqueMeshes.back().material.diffuseIdx = insertResult.first->second;
		}
	}

	// -- Specular --
	count = material->GetTextureCount(aiTextureType_SPECULAR);
	for (uint32_t mIdx{}; mIdx < count; ++mIdx)
	{
		aiString texturePath;
		material->GetTexture(aiTextureType_SPECULAR, mIdx, &texturePath);
		std::stringstream ss;
		ss << "textures/" << texturePath.C_Str();

		uint32_t idx = static_cast<uint32_t>(textures.size());
		auto insertResult = pathToIdx.insert({ ss.str(), idx });
		if (insertResult.second)
		{
			opaqueMeshes.back().material.specularIdx = idx;
			textures.emplace_back();
			textures.back().LoadFromFile(ss.str());
		}
		else
		{
			opaqueMeshes.back().material.specularIdx = insertResult.first->second;
		}
	}

	// -- Shininess --
	material->Get(AI_MATKEY_SHININESS, opaqueMeshes.back().material.exp);
	count = material->GetTextureCount(aiTextureType_SHININESS);
	for (uint32_t mIdx{}; mIdx < count; ++mIdx)
	{
		aiString texturePath;
		material->GetTexture(aiTextureType_SHININESS, mIdx, &texturePath);
		std::stringstream ss;
		ss << "textures/" << texturePath.C_Str();

		uint32_t idx = static_cast<uint32_t>(textures.size());
		auto insertResult = pathToIdx.insert({ ss.str(), idx });
		if (insertResult.second)
		{
			opaqueMeshes.back().material.shininessIdx = idx;
			textures.emplace_back();
			textures.back().LoadFromFile(ss.str());
		}
		else
		{
			opaqueMeshes.back().material.shininessIdx = insertResult.first->second;
		}
	}

	// -- Height --
	count = material->GetTextureCount(aiTextureType_HEIGHT);
	for (uint32_t mIdx{}; mIdx < count; ++mIdx)
	{
		aiString texturePath;
		material->GetTexture(aiTextureType_HEIGHT, mIdx, &texturePath);
		std::stringstream ss;
		ss << "textures/" << texturePath.C_Str();

		uint32_t idx = static_cast<uint32_t>(textures.size());
		auto insertResult = pathToIdx.insert({ ss.str(), idx });
		if (insertResult.second)
		{
			opaqueMeshes.back().material.heightIdx = idx;
			textures.emplace_back();
			textures.back().LoadFromFile(ss.str());
		}
		else
		{
			opaqueMeshes.back().material.heightIdx = insertResult.first->second;
		}
	}

	// -- Transparency --
	count = material->GetTextureCount(aiTextureType_OPACITY);
	if (count > 0)
	{
		transparentMeshes.push_back(opaqueMeshes.back());
		opaqueMeshes.pop_back();

		for (uint32_t mIdx{}; mIdx < count; ++mIdx)
		{
			aiString texturePath;
			material->GetTexture(aiTextureType_OPACITY, mIdx, &texturePath);
			std::stringstream ss;
			ss << "textures/" << texturePath.C_Str();

			uint32_t idx = static_cast<uint32_t>(textures.size());
			auto insertResult = pathToIdx.insert({ ss.str(), idx });
			if (insertResult.second)
			{
				transparentMeshes.back().material.opacityIdx = idx;
				textures.emplace_back();
				textures.back().LoadFromFile(ss.str());
			}
			else
			{
				transparentMeshes.back().material.opacityIdx = insertResult.first->second;
			}
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
	deletionQueue.Push([&] { vertexBuffer.Destroy(context); });
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
	deletionQueue.Push([&] { indexBuffer.Destroy(context); });
}
void pom::Model::CreateImages(const Context& context, CommandPool& cmdPool)
{
	uint32_t index = 0;
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
			.SetDebugName(std::ranges::find_if(pathToIdx, [&](auto& keyVal) { return keyVal.second == index; })->first.c_str())
			.SetWidth(texW)
			.SetHeight(texH)
			.SetFormat(VK_FORMAT_R8G8B8A8_SRGB)
			.SetTiling(VK_IMAGE_TILING_OPTIMAL)
			.SetMipLevels(mipLevels)
			.SetUsageFlags(VK_IMAGE_USAGE_SAMPLED_BIT)
			.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.InitialData(tex.GetPixels(), 0, texW, texH, tex.GetMemorySize(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmdPool)
			.Build(context, images.back());
		images.back().CreateView(context, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, mipLevels, 0, 1);
		++index;
	}
	deletionQueue.Push([&] { for (Image& image : images) image.Destroy(context); });
}
