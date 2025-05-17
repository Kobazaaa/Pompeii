// -- Pompeii Includes --
#include "DepthPrePass.h"
#include "Debugger.h"
#include "Shader.h"
#include "DescriptorPool.h"
#include "Context.h"
#include "Camera.h"
#include "Scene.h"

void pom::DepthPrePass::Initialize(const Context& context, const DepthPrePassCreateInfo& createInfo)
{
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
	}

	// -- Pipeline Layout --
	{
		GraphicsPipelineLayoutBuilder builder{};
		builder
			.NewPushConstantRange()
				.SetPCOffset(0)
				.SetPCSize(sizeof(PCModelDataVS))
				.SetPCStageFlags(VK_SHADER_STAGE_VERTEX_BIT)
			.AddLayout(m_UniformDSL)
			.Build(context, m_PipelineLayout);
		m_DeletionQueue.Push([&] {m_PipelineLayout.Destroy(context); });
	}

	// -- Pipelines --
	{
		// Load in shaders
		ShaderLoader shaderLoader{};
		ShaderModule vertShader;
		shaderLoader.Load(context, "shaders/depthprepass.vert.spv", vertShader);

		// Setup dynamic rendering info
		VkPipelineRenderingCreateInfo renderingCreateInfo{};
		renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingCreateInfo.depthAttachmentFormat = createInfo.depthFormat;

		// Create pipeline
		GraphicsPipelineBuilder builder{};
		builder
			.SetDebugName("Graphics Pipeline (Depth PrePass)")
			.SetPipelineLayout(m_PipelineLayout)
			.SetupDynamicRendering(renderingCreateInfo)
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.AddShader(vertShader, VK_SHADER_STAGE_VERTEX_BIT)
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetCullMode(VK_CULL_MODE_BACK_BIT)
			.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.SetPolygonMode(VK_POLYGON_MODE_FILL)
			.SetDepthTest(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)
			//.SetSampleCount(context.physicalDevice.GetMaxSampleCount())
			.SetVertexBindingDesc(Vertex::GetBindingDescription())
			.SetVertexAttributeDesc(Vertex::GetAttributeDescriptions())
			.Build(context, m_Pipeline);
		m_DeletionQueue.Push([&] { m_Pipeline.Destroy(context); });

		vertShader.Destroy(context);
	}

	// -- UBO --
	{
		m_vUniformBuffers.resize(createInfo.maxFramesInFlight);
		for (size_t i{}; i < createInfo.maxFramesInFlight; ++i)
		{
			BufferAllocator bufferAlloc{};
			bufferAlloc
				.SetDebugName("Uniform Buffer (Matrices) - Depth PrePass")
				.SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
				.SetSize(sizeof(UniformBufferVS))
				.HostAccess(true)
				.Allocate(context, m_vUniformBuffers[i]);
		}
		m_DeletionQueue.Push([&] { for (auto& ubo : m_vUniformBuffers) ubo.Destroy(context); });
	}

	// -- Buffers --
	{
		m_vUniformDS = createInfo.pDescriptorPool->AllocateSets(context, m_UniformDSL, createInfo.maxFramesInFlight, "Uniform Buffer DS");

		// -- Write UBO --
		DescriptorSetWriter writer{};
		for (size_t i{}; i < createInfo.maxFramesInFlight; ++i)
		{
			writer
				.AddBufferInfo(m_vUniformBuffers[i], 0, sizeof(UniformBufferVS))
				.WriteBuffers(m_vUniformDS[i], 0)
				.Execute(context);
		}
	}
}

void pom::DepthPrePass::Destroy()
{
	m_DeletionQueue.Flush();
}

void pom::DepthPrePass::Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, const Image& depthImage, Scene* pScene, Camera* pCamera) const
{
	// -- Update Vertex UBO --
	UniformBufferVS ubo;
	ubo.view = pCamera->GetViewMatrix();
	ubo.proj = pCamera->GetProjectionMatrix();
	vmaCopyMemoryToAllocation(context.allocator, &ubo, m_vUniformBuffers[imageIndex].GetMemoryHandle(), 0, sizeof(ubo));

	// -- Set Up Attachments --
	VkRenderingAttachmentInfo depthAttachment{};
	depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depthAttachment.imageView = depthImage.GetViewHandle();
	depthAttachment.imageLayout = depthImage.GetCurrentLayout();
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.clearValue.depthStencil = { .depth = 1.0f, .stencil = 0 };

	// -- Render Info --
	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea = VkRect2D{ VkOffset2D{0, 0}, depthImage.GetExtent2D() };
	renderingInfo.layerCount = 1;
	renderingInfo.pDepthAttachment = &depthAttachment;

	// -- Render --
	const VkCommandBuffer& vCmdBuffer = commandBuffer.GetHandle();
	Debugger::BeginDebugLabel(commandBuffer, "Depth Pre-Pass", glm::vec4(0.6f, 0.2f, 0.8f, 1));
	vkCmdBeginRendering(vCmdBuffer, &renderingInfo);
	{
		// -- Set Dynamic Viewport --
		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(depthImage.GetExtent2D().width);
		viewport.height = static_cast<float>(depthImage.GetExtent2D().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		Debugger::InsertDebugLabel(commandBuffer, "Bind Viewport", glm::vec4(0.2f, 1.f, 0.2f, 1.f));
		vkCmdSetViewport(vCmdBuffer, 0, 1, &viewport);

		// -- Set Dynamic Scissors --
		VkRect2D scissor;
		scissor.offset = { .x = 0, .y = 0 };
		scissor.extent = depthImage.GetExtent2D();
		Debugger::InsertDebugLabel(commandBuffer, "Bind Scissor", glm::vec4(1.f, 1.f, 0.2f, 1.f));
		vkCmdSetScissor(vCmdBuffer, 0, 1, &scissor);

		// -- Bind Descriptor Sets --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Uniform Buffer", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout.GetHandle(), 0, 1, &m_vUniformDS[imageIndex].GetHandle(), 0, nullptr);

		// -- Bind Pipeline --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Pipeline (Depth PrePass)", glm::vec4(0.2f, 0.4f, 1.f, 1.f));
		vkCmdBindPipeline(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetHandle());

		// -- Draw Models --
		for (const Model& model : pScene->GetModels())
		{
			// -- Bind Model Data --
			model.Bind(commandBuffer);

			// -- Draw Opaque --
			for (const Mesh& mesh : model.opaqueMeshes)
			{
				// -- Bind Push Constants --
				Debugger::InsertDebugLabel(commandBuffer, "Push Constants", glm::vec4(1.f, 0.6f, 0.f, 1.f));
				PCModelDataVS pcvs
				{
					.model = mesh.matrix
				};
				vkCmdPushConstants(vCmdBuffer, m_PipelineLayout.GetHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
					sizeof(PCModelDataVS), &pcvs);

				// -- Drawing Time! --
				vkCmdDrawIndexed(vCmdBuffer, mesh.indexCount, 1, mesh.indexOffset, mesh.vertexOffset, 0);
				Debugger::InsertDebugLabel(commandBuffer, "Draw Opaque Mesh - " + mesh.name, glm::vec4(0.4f, 0.8f, 1.f, 1.f));
			}
		}
	}
	vkCmdEndRendering(vCmdBuffer);
	Debugger::EndDebugLabel(commandBuffer);
}
