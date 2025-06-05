// -- Pompeii Includes --
#include "ShadowPass.h"
#include "Debugger.h"
#include "Model.h"
#include "Shader.h"
#include "Context.h"
#include "DescriptorPool.h"

//! This Shadow Pass is NO LONGER IN USE!!!
//! Shadows are handled differently now!

void pom::ShadowPass::Initialize(Context& context, const ShadowPassCreateInfo& createInfo)
{
	// -- Descriptor Set Layout --
	{
		DescriptorSetLayoutBuilder builder{};
		builder
			.SetDebugName("Light Data DSL")
			.NewLayoutBinding()
				.SetType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				.SetShaderStages(VK_SHADER_STAGE_VERTEX_BIT)
			.Build(context, m_LightDataDSL);
		m_DeletionQueue.Push([&] { m_LightDataDSL.Destroy(context); });
	}

	// -- Pipeline Layout --
	{
		PipelineLayoutBuilder builder{};
		builder
			.NewPushConstantRange()
				.SetPCOffset(0)
				.SetPCSize(sizeof(PCModelDataVS))
				.SetPCStageFlags(VK_SHADER_STAGE_VERTEX_BIT)
			.AddLayout(m_LightDataDSL)
			.Build(context, m_ShadowPipelineLayout);
		m_DeletionQueue.Push([&] {m_ShadowPipelineLayout.Destroy(context); });
	}

	// -- Pipeline --
	{
		ShaderLoader shaderLoader{};
		ShaderModule vertShader;
		shaderLoader.Load(context, "shaders/shadowmap.vert.spv", vertShader);

		// Setup dynamic rendering info
		VkPipelineRenderingCreateInfo renderingCreateInfo{};
		renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

		GraphicsPipelineBuilder builder{};
		builder
			.SetDebugName("Shadow Pipeline")
			.SetPipelineLayout(m_ShadowPipelineLayout)
			.SetupDynamicRendering(renderingCreateInfo)
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.AddShader(vertShader, VK_SHADER_STAGE_VERTEX_BIT)
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetCullMode(VK_CULL_MODE_FRONT_BIT)
			.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.EnableDepthBias(1.25f, 1.75f)
			.SetPolygonMode(VK_POLYGON_MODE_FILL)
			.SetDepthTest(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)
			.SetVertexBindingDesc(Vertex::GetBindingDescription())
			.SetVertexAttributeDesc(Vertex::GetAttributeDescriptions())
			.Build(context, m_ShadowPipeline);
		m_DeletionQueue.Push([&] { m_ShadowPipeline.Destroy(context); });

		vertShader.Destroy(context);
	}

	// -- Images --
	{
		m_vShadowMaps.resize(createInfo.maxFramesInFlight);
		for (Image& image : m_vShadowMaps)
		{
			ImageBuilder builder{};
			builder
				.SetDebugName("Shadow Map")
				.SetWidth(createInfo.extent.x)
				.SetHeight(createInfo.extent.y)
				.SetFormat(VK_FORMAT_D32_SFLOAT)
				.SetSampleCount(VK_SAMPLE_COUNT_1_BIT)
				.SetTiling(VK_IMAGE_TILING_OPTIMAL)
				.SetUsageFlags(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
				.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				.Build(context, image);
			image.CreateView(context, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);
			m_DeletionQueue.Push([&] { image.Destroy(context); });
		}
	}

	// -- Shadow Sampler --
	{
		SamplerBuilder builder{};
		builder
			.SetFilters(VK_FILTER_LINEAR, VK_FILTER_LINEAR)
			.SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
			.SetBorderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
			.Build(context, m_ShadowSampler);
		m_DeletionQueue.Push([&] { m_ShadowSampler.Destroy(context); });
	}

	// -- Buffers --
	{
		m_vLightDataBuffers.resize(createInfo.maxFramesInFlight);
		for (size_t i{}; i < createInfo.maxFramesInFlight; ++i)
		{
			BufferAllocator bufferAlloc{};
			bufferAlloc
				.SetDebugName("Light UBO")
				.SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
				.SetSize(sizeof(LightDataVS))
				.HostAccess(true)
				.Allocate(context, m_vLightDataBuffers[i]);
		}
		m_DeletionQueue.Push([&] { for (auto& ubo : m_vLightDataBuffers) ubo.Destroy(context); });


		m_vLightDataDS = context.descriptorPool->AllocateSets(context, m_LightDataDSL, createInfo.maxFramesInFlight, "Light Data DS");
		DescriptorSetWriter writer{};
		for (size_t i{}; i < createInfo.maxFramesInFlight; ++i)
		{
			writer
				.AddBufferInfo(m_vLightDataBuffers[i], 0, sizeof(LightDataVS))
				.WriteBuffers(m_vLightDataDS[i], 0)
				.Execute(context);
		}
	}
}

void pom::ShadowPass::Destroy()
{
	m_DeletionQueue.Flush();
}

void pom::ShadowPass::Record(const Context&, CommandBuffer&, uint32_t, Scene*)
{
	//LightDataVS light;
	//glm::vec3 dir = { 1, -1, 1 };
	//dir = normalize(dir);
	//light.lightSpace = pScene->directionalLight.GetLightSpaceMatrix();
	//vmaCopyMemoryToAllocation(context.allocator, &light, m_vLightDataBuffers[imageIndex].GetMemoryHandle(), 0, sizeof(light));

	//// Transition Image
	//m_vShadowMaps[imageIndex].TransitionLayout(commandBuffer,
	//	VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
	//	VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
	//	VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
	//	0, 1, 0, 1);

	//// Setup attachments
	//VkRenderingAttachmentInfo depthAttachment{};
	//depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	//depthAttachment.imageView = m_vShadowMaps[imageIndex].GetViewHandle();
	//depthAttachment.imageLayout = m_vShadowMaps[imageIndex].GetCurrentLayout();
	//depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//depthAttachment.clearValue.depthStencil = { .depth = 1.0f, .stencil = 0 };

	//// Render Info
	//VkRenderingInfo renderingInfo{};
	//renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	//renderingInfo.renderArea = VkRect2D{ VkOffset2D{0, 0}, m_vShadowMaps[imageIndex].GetExtent2D() };
	//renderingInfo.layerCount = 1;
	//renderingInfo.pDepthAttachment = &depthAttachment;

	//const VkCommandBuffer& vCmdBuffer = commandBuffer.GetHandle();
	//Debugger::BeginDebugLabel(commandBuffer, "Shadow Pass", glm::vec4(0.6f, 0.2f, 0.8f, 1));
	//vkCmdBeginRendering(vCmdBuffer, &renderingInfo);
	//{
	//	// -- Set Dynamic Viewport --
	//	VkViewport viewport;
	//	viewport.x = 0.0f;
	//	viewport.y = 0.0f;
	//	viewport.width = static_cast<float>(m_vShadowMaps[imageIndex].GetExtent2D().width);
	//	viewport.height = static_cast<float>(m_vShadowMaps[imageIndex].GetExtent2D().height);
	//	viewport.minDepth = 0.0f;
	//	viewport.maxDepth = 1.0f;
	//	Debugger::InsertDebugLabel(commandBuffer, "Bind Viewport", glm::vec4(0.2f, 1.f, 0.2f, 1.f));
	//	vkCmdSetViewport(vCmdBuffer, 0, 1, &viewport);

	//	// -- Set Dynamic Scissors --
	//	VkRect2D scissor;
	//	scissor.offset = { .x = 0, .y = 0 };
	//	scissor.extent = m_vShadowMaps[imageIndex].GetExtent2D();
	//	Debugger::InsertDebugLabel(commandBuffer, "Bind Scissor", glm::vec4(1.f, 1.f, 0.2f, 1.f));
	//	vkCmdSetScissor(vCmdBuffer, 0, 1, &scissor);

	//	// -- Bind Descriptor Sets --
	//	Debugger::InsertDebugLabel(commandBuffer, "Bind Uniform Buffer", glm::vec4(0.f, 1.f, 1.f, 1.f));
	//	vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowPipelineLayout.GetHandle(), 0, 1,
	//		&m_vLightDataDS[imageIndex].GetHandle(), 0, nullptr);

	//	// -- Bind Model Data --
	//	pScene->model.Bind(commandBuffer);

	//	// -- Draw Opaque --
	//	vkCmdBindPipeline(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowPipeline.GetHandle());
	//	Debugger::InsertDebugLabel(commandBuffer, "Bind Shadow Pipeline", glm::vec4(0.2f, 0.4f, 1.f, 1.f));
	//	for (const Mesh& mesh : pScene->model.opaqueMeshes)
	//	{
	//		PCModelDataVS pc
	//		{
	//			.model = mesh.matrix
	//		};
	//		vkCmdPushConstants(vCmdBuffer, m_ShadowPipelineLayout.GetHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
	//		                   sizeof(PCModelDataVS), &pc);

	//		vkCmdDrawIndexed(vCmdBuffer, mesh.indexCount, 1, mesh.indexOffset, mesh.vertexOffset, 0);
	//		Debugger::InsertDebugLabel(commandBuffer, "Draw Opaque Mesh - " + mesh.name, glm::vec4(0.4f, 0.8f, 1.f, 1.f));
	//	}
	//}
	//vkCmdEndRendering(vCmdBuffer);
	//Debugger::EndDebugLabel(commandBuffer);

	//// Transition Image
	//m_vShadowMaps[imageIndex].TransitionLayout(commandBuffer,
	//	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	//	VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
	//	VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
	//	0, 1, 0, 1);
}

const pom::Sampler& pom::ShadowPass::GetSampler() const
{
	return m_ShadowSampler;
}
pom::Image& pom::ShadowPass::GetMap(uint32_t idx)
{
	return m_vShadowMaps[idx];
}

