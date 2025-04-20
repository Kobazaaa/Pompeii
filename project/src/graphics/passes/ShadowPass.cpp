// -- Pompeii Includes --
#include "ShadowPass.h"
#include "Debugger.h"
#include "Model.h"
#include "Shader.h"
#include "Context.h"
#include "DescriptorPool.h"

void pom::ShadowPass::Initialize(const Context& context, const ShadowPassCreateInfo& createInfo)
{
	// -- Shadow Pass --
	{
		RenderPassBuilder builder{};
		builder
			.NewSubpass()
				.SetBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS)
				.NewAttachment()
				.SetFormat(VK_FORMAT_D32_SFLOAT)
				.SetSamples(VK_SAMPLE_COUNT_1_BIT)
				.SetLoadStoreOp(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
				.SetStencilLoadStoreOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE)
				.SetInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
				.SetFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
				.SetSubpassDepthAttachment(0)
			.NewDependency()
				.AddDependencyFlag(VK_DEPENDENCY_BY_REGION_BIT)
				.SetSrcSubPass(VK_SUBPASS_EXTERNAL)
				.SetDstSubPass(0)
				.SetSrcMasks(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT)
				.SetDstMasks(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
			.NewDependency()
				.AddDependencyFlag(VK_DEPENDENCY_BY_REGION_BIT)
				.SetSrcSubPass(0)
				.SetDstSubPass(VK_SUBPASS_EXTERNAL)
				.SetSrcMasks(VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
				.SetDstMasks(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT)
			.Build(context, m_ShadowPass);
		m_DeletionQueue.Push([&] { m_ShadowPass.Destroy(context); });
	}

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
		GraphicsPipelineLayoutBuilder builder{};
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

		GraphicsPipelineBuilder builder{};
		builder
			.SetDebugName("Shadow Pipeline")
			.SetPipelineLayout(m_ShadowPipelineLayout)
			.SetRenderPass(m_ShadowPass)
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.AddShader(vertShader, VK_SHADER_STAGE_VERTEX_BIT)
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetCullMode(VK_CULL_MODE_BACK_BIT)
			.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.SetColorWriteMask(0)
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
			image.CreateView(context, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);
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
			.EnableAnisotropy(context.physicalDevice.GetProperties().limits.maxSamplerAnisotropy)
			.Build(context, m_ShadowSampler);
		m_DeletionQueue.Push([&] { m_ShadowSampler.Destroy(context); });
	}

	// -- Shadow FrameBuffers --
	{
		for (const Image& image : m_vShadowMaps)
		{
			FrameBufferBuilder builder{};
			builder
				.SetRenderPass(m_ShadowPass)
				.AddAttachment(image.GetViewHandle())
				.SetExtent(createInfo.extent.x, createInfo.extent.y)
				.Build(context, m_vFrameBuffers);
		}
		m_DeletionQueue.Push([&] { for (auto& framebuffer : m_vFrameBuffers) framebuffer.Destroy(context); m_vFrameBuffers.clear(); });
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


		m_vLightDataDS = createInfo.pDescriptorPool->AllocateSets(context, m_LightDataDSL, createInfo.maxFramesInFlight, "Light Data DS");
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

void pom::ShadowPass::Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, Scene* pScene)
{
	LightDataVS light;
	glm::vec3 dir = { 1, -1, 1 };
	dir = normalize(dir);
	light.lightSpace = pScene->directionalLight.GetLightSpaceMatrix();
	vmaCopyMemoryToAllocation(context.allocator, &light, m_vLightDataBuffers[imageIndex].GetMemoryHandle(), 0, sizeof(light));

	const VkCommandBuffer& vCmdBuffer = commandBuffer.GetHandle();
	VkRenderPassBeginInfo shadowPassInfo{};
	shadowPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	shadowPassInfo.renderPass = m_ShadowPass.GetHandle();
	shadowPassInfo.framebuffer = m_vFrameBuffers[imageIndex].GetHandle();
	shadowPassInfo.renderArea.offset = { .x = 0, .y = 0 };
	shadowPassInfo.renderArea.extent = m_vFrameBuffers[imageIndex].GetExtent();

	VkClearValue clearValue{};
	clearValue.depthStencil = { .depth = 1.0f, .stencil = 0 };
	shadowPassInfo.clearValueCount = 1;
	shadowPassInfo.pClearValues = &clearValue;

	Debugger::BeginDebugLabel(commandBuffer, "Shadow Pass", glm::vec4(0.6f, 0.2f, 0.8f, 1));
	vkCmdBeginRenderPass(vCmdBuffer, &shadowPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	{
		// -- Set Dynamic Viewport --
		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_vFrameBuffers[imageIndex].GetExtent().width);
		viewport.height = static_cast<float>(m_vFrameBuffers[imageIndex].GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		Debugger::InsertDebugLabel(commandBuffer, "Bind Viewport", glm::vec4(0.2f, 1.f, 0.2f, 1.f));
		vkCmdSetViewport(vCmdBuffer, 0, 1, &viewport);

		// -- Set Dynamic Scissors --
		VkRect2D scissor;
		scissor.offset = { .x = 0, .y = 0 };
		scissor.extent = m_vFrameBuffers[imageIndex].GetExtent();
		Debugger::InsertDebugLabel(commandBuffer, "Bind Scissor", glm::vec4(1.f, 1.f, 0.2f, 1.f));
		vkCmdSetScissor(vCmdBuffer, 0, 1, &scissor);

		// -- Bind Descriptor Sets --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Uniform Buffer", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowPipelineLayout.GetHandle(), 0, 1,
			&m_vLightDataDS[imageIndex].GetHandle(), 0, nullptr);

		// -- Bind Model Data --
		pScene->model.Bind(commandBuffer);

		// -- Draw Opaque --
		vkCmdBindPipeline(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowPipeline.GetHandle());
		Debugger::InsertDebugLabel(commandBuffer, "Bind Shadow Pipeline", glm::vec4(0.2f, 0.4f, 1.f, 1.f));
		for (const Mesh& mesh : pScene->model.opaqueMeshes)
		{
			PCModelDataVS pc
			{
				.model = mesh.matrix
			};
			vkCmdPushConstants(vCmdBuffer, m_ShadowPipelineLayout.GetHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
			                   sizeof(PCModelDataVS), &pc);

			vkCmdDrawIndexed(vCmdBuffer, mesh.indexCount, 1, mesh.indexOffset, mesh.vertexOffset, 0);
			Debugger::InsertDebugLabel(commandBuffer, "Draw Opaque Mesh - " + mesh.name, glm::vec4(0.4f, 0.8f, 1.f, 1.f));
		}
	}
	vkCmdEndRenderPass(vCmdBuffer);
	Debugger::EndDebugLabel(commandBuffer);
}

const pom::Sampler& pom::ShadowPass::GetSampler() const
{
	return m_ShadowSampler;
}

const pom::Image& pom::ShadowPass::GetMap(uint32_t idx) const
{
	return m_vShadowMaps[idx];
}

