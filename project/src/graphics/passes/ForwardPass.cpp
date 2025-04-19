// -- Standard Library --
#include <array>
#include <iostream>

// -- Pompeii Includes --
#include "ForwardPass.h"
#include "ShadowPass.h"
#include "Debugger.h"
#include "Shader.h"
#include "SwapChain.h"
#include "DescriptorPool.h"
#include "FrameBuffer.h"
#include "Context.h"
#include "Camera.h"

void pom::ForwardPass::Initialize(const Context& context, const ForwardPassCreateInfo& createInfo)
{
	// -- Render Pass --
	{
		RenderPassBuilder builder{};

		builder
			.NewSubpass()
				.SetBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS)
				.NewAttachment()
					.SetFormat(createInfo.swapChain->GetFormat())
					.SetSamples(context.physicalDevice.GetMaxSampleCount())
					.SetLoadStoreOp(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
					.SetStencilLoadStoreOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE)
					.SetInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
					.SetFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
					.AddSubpassColorAttachment(0)
				.NewAttachment()
					.SetFormat(createInfo.swapChain->GetDepthImage().GetFormat())
					.SetSamples(context.physicalDevice.GetMaxSampleCount())
					.SetLoadStoreOp(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE)
					.SetStencilLoadStoreOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE)
					.SetInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
					.SetFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
					.SetSubpassDepthAttachment(1)
				.NewAttachment()
					.SetFormat(createInfo.swapChain->GetFormat())
					.SetSamples(VK_SAMPLE_COUNT_1_BIT)
					.SetLoadStoreOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE)
					.SetStencilLoadStoreOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE)
					.SetInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
					.SetFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
					.SetSubpassResolveAttachment(2)
				.NewDependency()
					.SetSrcSubPass(VK_SUBPASS_EXTERNAL)
					.SetDstSubPass(0)
					.SetSrcMasks(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0)
					.SetDstMasks(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
						VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
			.Build(context, m_ForwardPass);
		m_DeletionQueue.Push([&] { m_ForwardPass.Destroy(context); });
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
		builder
			.SetDebugName("Texture Array DS Layout")
			.NewLayoutBinding()
			.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.SetCount(static_cast<uint32_t>(createInfo.model->images.size()))
			.Build(context, m_TextureDSL);
		m_DeletionQueue.Push([&] { m_TextureDSL.Destroy(context); });

		// -- Shadow Map --

		builder
			.SetDebugName("LightMap")
			.NewLayoutBinding()
			.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(context, m_ShadowMapDSL);
		m_DeletionQueue.Push([&] { m_ShadowMapDSL.Destroy(context); });
	}

	// -- Pipeline Layout --
	{
		GraphicsPipelineLayoutBuilder builder{};

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
			.AddLayout(m_ShadowMapDSL)
			.Build(context, m_DefaultPipelineLayout);
		m_DeletionQueue.Push([&] {m_DefaultPipelineLayout.Destroy(context); });
	}

	// -- Pipelines --
	{
		ShaderLoader shaderLoader{};

		ShaderModule vertShader;
		ShaderModule fragShader;
		shaderLoader.Load(context, "shaders/shader.vert.spv", vertShader);
		shaderLoader.Load(context, "shaders/shader.frag.spv", fragShader);

		uint32_t arraySize = static_cast<uint32_t>(createInfo.model->images.size());
		GraphicsPipelineBuilder builder{};
		builder
			.SetDebugName("Graphics Pipeline (Default)")
			.SetPipelineLayout(m_DefaultPipelineLayout)
			.SetRenderPass(m_ForwardPass)
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.AddShader(vertShader, VK_SHADER_STAGE_VERTEX_BIT)
			.AddShader(fragShader, VK_SHADER_STAGE_FRAGMENT_BIT)
				.SetShaderSpecialization(0, 0, sizeof(uint32_t), &arraySize)
			.EnableSampleShading(0.2f)
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetCullMode(VK_CULL_MODE_BACK_BIT)
			.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.SetPolygonMode(VK_POLYGON_MODE_FILL)
			.SetSampleCount(context.physicalDevice.GetMaxSampleCount())
			.SetDepthTest(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)
			.SetVertexBindingDesc(Vertex::GetBindingDescription())
			.SetVertexAttributeDesc(Vertex::GetAttributeDescriptions())
			.Build(context, m_OpaquePipeline);
		m_DeletionQueue.Push([&] { m_OpaquePipeline.Destroy(context); });

		builder = {};

		builder
			.SetDebugName("Graphics Pipeline (Transparent)")
			.SetPipelineLayout(m_DefaultPipelineLayout)
			.SetRenderPass(m_ForwardPass)
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.AddShader(vertShader, VK_SHADER_STAGE_VERTEX_BIT)
			.AddShader(fragShader, VK_SHADER_STAGE_FRAGMENT_BIT)
				.SetShaderSpecialization(0, 0, sizeof(uint32_t), &arraySize)
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetCullMode(VK_CULL_MODE_NONE)
			.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.SetPolygonMode(VK_POLYGON_MODE_FILL)
			.SetSampleCount(context.physicalDevice.GetMaxSampleCount())
			.SetDepthTest(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)
			.SetVertexBindingDesc(Vertex::GetBindingDescription())
			.SetVertexAttributeDesc(Vertex::GetAttributeDescriptions())
			.EnableBlend()
			.SetColorBlend(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD)
			.SetAlphaBlend(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD)
			.Build(context, m_TransparentPipeline);
		m_DeletionQueue.Push([&] { m_TransparentPipeline.Destroy(context); });

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
			.Build(context, m_Sampler);
		m_DeletionQueue.Push([&] { m_Sampler.Destroy(context); });
	}

	// -- UBO --
	{
		m_vUniformBuffers.resize(createInfo.maxFramesInFlight);
		for (size_t i{}; i < createInfo.maxFramesInFlight; ++i)
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
		m_vUniformDS = createInfo.descriptorPool->AllocateSets(context, m_UniformDSL, createInfo.maxFramesInFlight, "Uniform Buffer DS");
		m_TextureDS = createInfo.descriptorPool->AllocateSets(context, m_TextureDSL, 1, "Texture Array DS").front();
		m_ShadowMapDS = createInfo.descriptorPool->AllocateSets(context, m_ShadowMapDSL, createInfo.maxFramesInFlight, "Shadow Map DS");

		// -- Write UBO --
		DescriptorSetWriter writer{};
		for (size_t i{}; i < createInfo.maxFramesInFlight; ++i)
		{
			writer
				.AddBufferInfo(m_vUniformBuffers[i], 0, sizeof(UniformBufferVS))
				.WriteBuffers(m_vUniformDS[i], 0)
				.Execute(context);

			writer
				.AddImageInfo(createInfo.shadowPass->GetMap(static_cast<uint32_t>(i)),
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, createInfo.shadowPass->GetSampler())
				.WriteImages(m_ShadowMapDS[i], 0)
				.Execute(context);
		}

		// -- Write Textures --
		for (Image& image : createInfo.model->images)
		{
			writer.AddImageInfo(image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_Sampler);
		}
		writer.WriteImages(m_TextureDS, 0).Execute(context);
	}
}

void pom::ForwardPass::Destroy()
{
	m_DeletionQueue.Flush();
}

void pom::ForwardPass::Record(const Context& context, const FrameBuffer& fb, const SwapChain& sc, CommandBuffer& commandBuffer, uint32_t imageIndex, const Model& model, Camera* pCamera)
{
	UniformBufferVS ubo;
	ubo.view = pCamera->GetViewMatrix();
	ubo.proj = pCamera->GetProjectionMatrix();
	ubo.viewL = lookAtLH(glm::vec3(-1000, 1000, -1000), glm::vec3(0), glm::vec3(0.f, 1.f, 0.f));
	ubo.projL = glm::orthoLH(-2048.f, 2048.f, -2048.f, 2048.f, 0.1f, 3'000.f);

	vmaCopyMemoryToAllocation(context.allocator, &ubo, m_vUniformBuffers[imageIndex].GetMemoryHandle(), 0, sizeof(ubo));

	const VkCommandBuffer& vCmdBuffer = commandBuffer.GetHandle();
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_ForwardPass.GetHandle();
	renderPassInfo.framebuffer = fb.GetHandle();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = sc.GetExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.53f, 0.81f, 0.92f, 1.0f} };
	clearValues[1].depthStencil = { .depth = 1.0f, .stencil = 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	Debugger::BeginDebugLabel(commandBuffer, "Render Pass", glm::vec4(0.6f, 0.2f, 0.8f, 1));
	vkCmdBeginRenderPass(vCmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	{
		// -- Set Dynamic Viewport --
		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(sc.GetExtent().width);
		viewport.height = static_cast<float>(sc.GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		Debugger::InsertDebugLabel(commandBuffer, "Bind Viewport", glm::vec4(0.2f, 1.f, 0.2f, 1.f));
		vkCmdSetViewport(vCmdBuffer, 0, 1, &viewport);

		// -- Set Dynamic Scissors --
		VkRect2D scissor;
		scissor.offset = { .x = 0, .y = 0 };
		scissor.extent = sc.GetExtent();
		Debugger::InsertDebugLabel(commandBuffer, "Bind Scissor", glm::vec4(1.f, 1.f, 0.2f, 1.f));
		vkCmdSetScissor(vCmdBuffer, 0, 1, &scissor);

		// -- Bind Descriptor Sets --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Uniform Buffer", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DefaultPipelineLayout.GetHandle(), 0, 1, &m_vUniformDS[imageIndex].GetHandle(), 0, nullptr);

		Debugger::InsertDebugLabel(commandBuffer, "Bind Textures", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DefaultPipelineLayout.GetHandle(), 1, 1, &m_TextureDS.GetHandle(), 0, nullptr);

		Debugger::InsertDebugLabel(commandBuffer, "Bind Shadow Map", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DefaultPipelineLayout.GetHandle(), 2, 1, &m_ShadowMapDS[imageIndex].GetHandle(), 0, nullptr);

		// -- Bind Model Data --
		model.Bind(commandBuffer);

		// -- Draw Opaque --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Pipeline (Default)", glm::vec4(0.2f, 0.4f, 1.f, 1.f));
		vkCmdBindPipeline(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_OpaquePipeline.GetHandle());
		for (const Mesh& mesh : model.opaqueMeshes)
		{
			// -- Bind Push Constants --
			Debugger::InsertDebugLabel(commandBuffer, "Push Constants", glm::vec4(1.f, 0.6f, 0.f, 1.f));
			PCModelDataVS pcvs
			{
				.model = mesh.matrix
			};
			vkCmdPushConstants(vCmdBuffer, m_DefaultPipelineLayout.GetHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
			                   sizeof(PCModelDataVS), &pcvs);

			PCMaterialDataFS pcfs
			{
				.diffuseIdx = mesh.material.diffuseIdx,
				.opacityIdx = mesh.material.opacityIdx,
				.specularIdx = mesh.material.specularIdx,
				.shininessIdx = mesh.material.shininessIdx,
				.heightIdx = mesh.material.heightIdx,
				.expo = mesh.material.exp
			};
			vkCmdPushConstants(vCmdBuffer, m_DefaultPipelineLayout.GetHandle(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PCModelDataVS),
			                   sizeof(PCMaterialDataFS), &pcfs);

			// -- Drawing Time! --
			vkCmdDrawIndexed(vCmdBuffer, mesh.indexCount, 1, mesh.indexOffset, mesh.vertexOffset, 0);
			Debugger::InsertDebugLabel(commandBuffer, "Draw Opaque Mesh - " + mesh.name, glm::vec4(0.4f, 0.8f, 1.f, 1.f));
		}

		// -- Draw Transparent --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Pipeline (Transparent)", glm::vec4(0.2f, 0.4f, 1.f, 1.f));
		vkCmdBindPipeline(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_TransparentPipeline.GetHandle());
		for (const Mesh& mesh : model.transparentMeshes)
		{
			// -- Bind Push Constants --
			Debugger::InsertDebugLabel(commandBuffer, "Push Constants", glm::vec4(1.f, 0.6f, 0.f, 1.f));
			PCModelDataVS pcvs
			{
				.model = mesh.matrix
			};
			vkCmdPushConstants(vCmdBuffer, m_DefaultPipelineLayout.GetHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
				sizeof(PCModelDataVS), &pcvs);

			PCMaterialDataFS pcfs
			{
				.diffuseIdx = mesh.material.diffuseIdx,
				.opacityIdx = mesh.material.opacityIdx,
				.specularIdx = mesh.material.specularIdx,
				.shininessIdx = mesh.material.shininessIdx,
				.heightIdx = mesh.material.heightIdx,
				.expo = mesh.material.exp
			};
			vkCmdPushConstants(vCmdBuffer, m_DefaultPipelineLayout.GetHandle(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PCModelDataVS),
				sizeof(PCMaterialDataFS), &pcfs);

			// -- Drawing Time! --
			vkCmdDrawIndexed(vCmdBuffer, mesh.indexCount, 1, mesh.indexOffset, mesh.vertexOffset, 0);
			Debugger::InsertDebugLabel(commandBuffer, "Draw Transparent Mesh - " + mesh.name, glm::vec4(0.4f, 0.8f, 1.f, 1.f));
		}
	}
	vkCmdEndRenderPass(vCmdBuffer);
	Debugger::EndDebugLabel(commandBuffer);
}

const pom::RenderPass& pom::ForwardPass::GetRenderPass() const
{
	return m_ForwardPass;
}
