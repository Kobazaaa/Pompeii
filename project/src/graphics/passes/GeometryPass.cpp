// -- Pompeii Includes --
#include "GeometryPass.h"
#include "Shader.h"
#include "Context.h"
#include "Debugger.h"
#include "DescriptorPool.h"
#include "RenderingItems.h"
#include "GPUCamera.h"

void pompeii::GeometryPass::Initialize(const Context& context, const GeometryPassCreateInfo& createInfo)
{
	// -- GBuffers --
	{
		m_vGBuffers.resize(context.maxFramesInFlight);
		for (GBuffer& gBuffer : m_vGBuffers)
			gBuffer.Initialize(context, createInfo.extent);
		m_DeletionQueue.Push([&] { for (GBuffer& gBuffer : m_vGBuffers) gBuffer.Destroy(context); });
	}

	// -- Descriptor Set Layout --
	{
		DescriptorSetLayoutBuilder builder{};

		// -- Uniform Buffer Descriptor --
		builder
			.SetDebugName("Uniform Buffer DS Layout")
			.NewLayoutBinding()
			.SetType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			.SetShaderStages(VK_SHADER_STAGE_VERTEX_BIT)
			.Build(context, m_UniformDSL);
		m_DeletionQueue.Push([&] { m_UniformDSL.Destroy(context); });

		// -- Texture Array Descriptor --
		constexpr uint32_t textureCount = 256;
		assert(textureCount <= context.physicalDevice.GetProperties().limits.maxDescriptorSetSampledImages && "GPU can't support this many sampled images!");
		builder
			.SetDebugName("Texture Array DS Layout")
			.NewLayoutBinding()
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
				.SetCount(textureCount)
				.AddLayoutFlag(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
				.AddBindingFlags(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT)
				.AddBindingFlags(VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)
				.AddBindingFlags(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
			.Build(context, m_TextureDSL);
		m_DeletionQueue.Push([&] { m_TextureDSL.Destroy(context); });
	}

	// -- Pipeline Layout --
	{
		PipelineLayoutBuilder builder{};

		builder
			.NewPushConstantRange()
				.SetPCOffset(0)
				.SetPCSize(sizeof(PCModelDataVS))
				.SetPCStageFlags(VK_SHADER_STAGE_VERTEX_BIT)
			.NewPushConstantRange()
				.SetPCOffset(sizeof(PCModelDataVS))
				.SetPCSize(sizeof(PCMaterialDataFS))
				.SetPCStageFlags(VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddLayout(m_UniformDSL)
			.AddLayout(m_TextureDSL)
			.Build(context, m_PipelineLayout);
		m_DeletionQueue.Push([&] {m_PipelineLayout.Destroy(context); });
	}

	// -- Pipelines --
	{
		// Load in shaders
		ShaderLoader shaderLoader{};
		ShaderModule vertShader;
		ShaderModule fragShader;
		shaderLoader.Load(context, "shaders/deferred.vert.spv", vertShader);
		shaderLoader.Load(context, "shaders/deferred.frag.spv", fragShader);

		// Setup dynamic rendering info
		VkPipelineRenderingCreateInfo renderingCreateInfo{};
		auto formats = m_vGBuffers[0].GetAllFormats();
		renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingCreateInfo.colorAttachmentCount = static_cast<uint32_t>(formats.size());
		renderingCreateInfo.pColorAttachmentFormats = formats.data();
		renderingCreateInfo.depthAttachmentFormat = createInfo.depthFormat;

		// Create pipeline
		GraphicsPipelineBuilder builder{};
		builder
			.SetDebugName("Graphics Pipeline (GBuffer)")
			.SetPipelineLayout(m_PipelineLayout)
			.SetupDynamicRendering(renderingCreateInfo)
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.AddShader(vertShader, VK_SHADER_STAGE_VERTEX_BIT)
			.AddShader(fragShader, VK_SHADER_STAGE_FRAGMENT_BIT)
			.EnableSampleShading(0.2f)
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetCullMode(VK_CULL_MODE_BACK_BIT)
			.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.SetPolygonMode(VK_POLYGON_MODE_FILL)
			.SetDepthTest(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL)
			.SetVertexBindingDesc(Vertex::GetBindingDescription())
			.SetVertexAttributeDesc(Vertex::GetAttributeDescriptions())
			.Build(context, m_Pipeline);
		m_DeletionQueue.Push([&] { m_Pipeline.Destroy(context); });

		fragShader.Destroy(context);
		vertShader.Destroy(context);
	}

	// -- Sampler --
	{
		SamplerBuilder builder{};
		builder
			.SetFilters(VK_FILTER_LINEAR, VK_FILTER_LINEAR)
			.SetAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
			.EnableAnisotropy(context.physicalDevice.GetProperties().limits.maxSamplerAnisotropy)
			.SetMipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
			.SetMipLevels(0.f, 0.f, VK_LOD_CLAMP_NONE)
			.SetBorderColor(VK_BORDER_COLOR_INT_OPAQUE_BLACK)
			.Build(context, m_TextureSampler);
		m_DeletionQueue.Push([&] { m_TextureSampler.Destroy(context); });
	}

	// -- UBO --
	{
		m_vUniformBuffers.resize(context.maxFramesInFlight);
		for (size_t i{}; i < context.maxFramesInFlight; ++i)
		{
			BufferAllocator bufferAlloc{};
			bufferAlloc
				.SetDebugName("Uniform Buffer (Matrices)")
				.SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
				.SetSize(sizeof(UniformBufferVS))
				.HostAccess(true)
				.Allocate(context, m_vUniformBuffers[i]);
		}
		m_DeletionQueue.Push([&] { for (auto& ubo : m_vUniformBuffers) ubo.Destroy(context); });
	}

	// -- Buffers --
	{
		m_vUniformDS = context.descriptorPool->AllocateSets(context, m_UniformDSL, context.maxFramesInFlight, "Uniform Buffer DS");

		// -- Write UBO --
		DescriptorSetWriter writer{};
		for (size_t i{}; i < context.maxFramesInFlight; ++i)
		{
			writer
				.AddBufferInfo(m_vUniformBuffers[i], 0, sizeof(UniformBufferVS))
				.WriteBuffers(m_vUniformDS[i], 0)
				.Execute(context);
		}
	}
}

void pompeii::GeometryPass::Destroy()
{
	m_DeletionQueue.Flush();
}

void pompeii::GeometryPass::Resize(const Context& context, VkExtent2D extent)
{
	for (GBuffer& gBuffer : m_vGBuffers)
		gBuffer.Resize(context, extent);
}
void pompeii::GeometryPass::UpdateTextureDescriptor(const Context& context, const std::vector<Image*>& vTextures)
{
	if (m_TextureDS.GetHandle())
		vkFreeDescriptorSets(context.device.GetHandle(), context.descriptorPool->GetHandle(), 1, &m_TextureDS.GetHandle());

	auto imageCount = static_cast<uint32_t>(vTextures.size());
	const uint32_t variableCount = imageCount;
	VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
	variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
	variableCountInfo.descriptorSetCount = 1;
	variableCountInfo.pDescriptorCounts = &variableCount;
	m_TextureDS = context.descriptorPool->AllocateSets(context, m_TextureDSL, 1, "Texture Array DS", &variableCountInfo).front();

	// -- Write Textures --
	if (imageCount <= 0)
		return;

	DescriptorSetWriter writer{};
	for (const Image* image : vTextures)
		writer.AddImageInfo(image->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_TextureSampler);
	writer.WriteImages(m_TextureDS, 0, imageCount).Execute(context);
	m_TextureCount = imageCount;
}

void pompeii::GeometryPass::UpdateCamera(const Context& context, uint32_t imageIndex, const CameraData& camera) const
{
	UniformBufferVS ubo;
	ubo.view = camera.view;
	ubo.proj = camera.proj;
	vmaCopyMemoryToAllocation(context.allocator, &ubo, m_vUniformBuffers[imageIndex].GetMemoryHandle(), 0, sizeof(ubo));
}
void pompeii::GeometryPass::Record(CommandBuffer& commandBuffer, uint32_t imageIndex, const Image& depthImage, const std::vector<RenderItem>& renderItems)
{
	// Transition GBuffer Images
	m_vGBuffers[imageIndex].TransitionBufferWriting(commandBuffer);

	// Setup attachments
	auto& gBufferAttachments = m_vGBuffers[imageIndex].GetRenderingAttachments();
	uint32_t gBufferAttachmentCount = m_vGBuffers[imageIndex].GetAttachmentCount();
	VkRenderingAttachmentInfo depthAttachment{};
	depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depthAttachment.imageView = depthImage.GetView().GetHandle();
	depthAttachment.imageLayout = depthImage.GetCurrentLayout();
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// Render Info
	VkExtent2D extent = m_vGBuffers[imageIndex].GetExtent();
	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea = VkRect2D{ VkOffset2D{0, 0}, extent };
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = gBufferAttachmentCount;
	renderingInfo.pColorAttachments = gBufferAttachments.data();
	renderingInfo.pDepthAttachment = &depthAttachment;
	renderingInfo.pStencilAttachment = nullptr;

	// Render
	const VkCommandBuffer& vCmdBuffer = commandBuffer.GetHandle();
	Debugger::BeginDebugLabel(commandBuffer, "Geometry Pass", glm::vec4(0.6f, 0.2f, 0.8f, 1));
	vkCmdBeginRendering(vCmdBuffer, &renderingInfo);
	{
		// -- Set Dynamic Viewport --
		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		Debugger::InsertDebugLabel(commandBuffer, "Bind Viewport", glm::vec4(0.2f, 1.f, 0.2f, 1.f));
		vkCmdSetViewport(vCmdBuffer, 0, 1, &viewport);

		// -- Set Dynamic Scissors --
		VkRect2D scissor;
		scissor.offset = { .x = 0, .y = 0 };
		scissor.extent = extent;
		Debugger::InsertDebugLabel(commandBuffer, "Bind Scissor", glm::vec4(1.f, 1.f, 0.2f, 1.f));
		vkCmdSetScissor(vCmdBuffer, 0, 1, &scissor);

		// -- Bind Descriptor Sets --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Uniform Buffer", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout.GetHandle(), 0, 1, &m_vUniformDS[imageIndex].GetHandle(), 0, nullptr);

		Debugger::InsertDebugLabel(commandBuffer, "Bind Textures", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout.GetHandle(), 1, 1, &m_TextureDS.GetHandle(), 0, nullptr);

		// -- Bind Pipeline --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Pipeline (GBuffer)", glm::vec4(0.2f, 0.4f, 1.f, 1.f));
		vkCmdBindPipeline(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetHandle());

		// -- Draw Models --
		for (uint32_t itemIdx{}; itemIdx < renderItems.size(); ++itemIdx)
		{
			const RenderItem& item = renderItems[itemIdx];
			Mesh* pMesh = item.mesh;

			// -- Bind Model Data --
			pMesh->Bind(commandBuffer);

			// -- Draw Opaque --
			for (const SubMesh& subMesh : pMesh->vSubMeshes)
			{
				// -- Bind Push Constants --
				Debugger::InsertDebugLabel(commandBuffer, "Push Constants", glm::vec4(1.f, 0.6f, 0.f, 1.f));
				PCModelDataVS pcvs
				{
					.model = item.transform * subMesh.matrix
				};
				vkCmdPushConstants(vCmdBuffer, m_PipelineLayout.GetHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
					sizeof(PCModelDataVS), &pcvs);

				uint32_t offset = itemIdx > 0 ? static_cast<uint32_t>(renderItems[itemIdx - 1].mesh->images.size()) : 0;
				auto applyOffset = [offset](uint32_t idx) {
					return idx == 0xFFFFFFFF ? idx : idx + offset;
					};
				PCMaterialDataFS pcfs{
					.diffuseIdx = applyOffset(subMesh.material.albedoIdx),
					.opacityIdx = applyOffset(subMesh.material.opacityIdx),
					.normalIdx = applyOffset(subMesh.material.normalIdx),
					.roughnessIdx = applyOffset(subMesh.material.roughnessIdx),
					.metallicIdx = applyOffset(subMesh.material.metalnessIdx),
					.textureCount = m_TextureCount,
				};
				vkCmdPushConstants(vCmdBuffer, m_PipelineLayout.GetHandle(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PCModelDataVS),
					sizeof(PCMaterialDataFS), &pcfs);

				// -- Drawing Time! --
				vkCmdDrawIndexed(vCmdBuffer, subMesh.indexCount, 1, subMesh.indexOffset, subMesh.vertexOffset, 0);
				Debugger::InsertDebugLabel(commandBuffer, "Draw Mesh - " + subMesh.name, glm::vec4(0.4f, 0.8f, 1.f, 1.f));
			}
		}
	}
	vkCmdEndRendering(vCmdBuffer);
	Debugger::EndDebugLabel(commandBuffer);

	m_vGBuffers[imageIndex].TransitionBufferSampling(commandBuffer);
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const std::vector<pompeii::GBuffer>& pompeii::GeometryPass::GetGBuffers() const						{ return m_vGBuffers; }
const pompeii::GBuffer& pompeii::GeometryPass::GetGBuffer(uint32_t index) const						{ return m_vGBuffers.at(index); }
uint32_t pompeii::GeometryPass::GetBoundTextureCount() const										{ return m_TextureCount; }
const pompeii::DescriptorSet& pompeii::GeometryPass::GetTexturesDescriptorSet() const				{ return m_TextureDS; }
const pompeii::DescriptorSetLayout& pompeii::GeometryPass::GetTexturesDescriptorSetLayout() const	{ return m_TextureDSL; }
