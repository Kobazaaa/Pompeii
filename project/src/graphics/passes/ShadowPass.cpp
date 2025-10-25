// -- Pompeii Includes --
#include "ShadowPass.h"
#include "Debugger.h"
#include "Shader.h"
#include "Context.h"
#include "Light.h"
#include "RenderingItems.h"

void pompeii::ShadowPass::Initialize(const Context& context)
{
	// -- Pipeline Layout --
	{
		PipelineLayoutBuilder pipelineLayoutBuilder{};
		pipelineLayoutBuilder
			.NewPushConstantRange()
			.SetPCSize(sizeof(PushConstants))
			.SetPCStageFlags(VK_SHADER_STAGE_VERTEX_BIT)
			.Build(context, m_ShadowPipelineLayout);
		m_DeletionQueue.Push([&] {m_ShadowPipelineLayout.Destroy(context); });
	}

	// -- Pipeline --
	{
		ShaderLoader shaderLoader{};
		ShaderModule vertShader;
		//ShaderModule fragShader;
		shaderLoader.Load(context, "shaders/shadowmap.vert.spv", vertShader);
		//shaderLoader.Load(context, "frag", fragShader);

		VkPipelineRenderingCreateInfo renderingCreateInfo{};
		VkFormat format = VK_FORMAT_D32_SFLOAT;
		renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingCreateInfo.depthAttachmentFormat = format;

		GraphicsPipelineBuilder pipelineBuilder{};
		pipelineBuilder
			.SetDebugName("Graphics Pipeline (Generate Light Depth Map)")
			.SetPipelineLayout(m_ShadowPipelineLayout)
			.SetupDynamicRendering(renderingCreateInfo)
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.AddShader(vertShader, VK_SHADER_STAGE_VERTEX_BIT)
			//.AddShader(fragShader, VK_SHADER_STAGE_FRAGMENT_BIT)
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetCullMode(VK_CULL_MODE_FRONT_BIT)
			.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.SetPolygonMode(VK_POLYGON_MODE_FILL)
			.EnableDepthBias(1.25f, 1.75f)
			.SetVertexAttributeDesc(Vertex::GetAttributeDescriptions())
			.SetVertexBindingDesc(Vertex::GetBindingDescription())
			.SetDepthTest(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL)
			.Build(context, m_ShadowPipeline);
		m_DeletionQueue.Push([&] { m_ShadowPipeline.Destroy(context); });

		//fragShader.Destroy(context);
		vertShader.Destroy(context);
	}
}

void pompeii::ShadowPass::Destroy()
{
	m_DeletionQueue.Flush();
}

void pompeii::ShadowPass::Record(const Context& context, CommandBuffer& commandBuffer, const std::vector<RenderItem>& renderItems, const std::vector<LightItem>& lightItems) const
{
	Debugger::BeginDebugLabel(commandBuffer, "Shadow Pass", glm::vec4(0.6f, 0.2f, 0.8f, 1));
	for (const LightItem& lightItem : lightItems)
	{
		auto& map = lightItem.light->vShadowMaps[context.currentFrame];
		auto extent = map.GetExtent2D();

		// -- Ready outImage to be rendered to --
		map.TransitionLayout(commandBuffer,
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
			0, map.GetMipLevels(), 0, map.GetLayerCount());

		// -- Render --
		const VkCommandBuffer& vCmd = commandBuffer.GetHandle();
		for (uint32_t layerIdx{1}; layerIdx < map.GetViewCount(); ++layerIdx)
		{
			// -- Setup Attachment --
			VkRenderingAttachmentInfo depthAttachment{};
			depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depthAttachment.imageView = map.GetView(layerIdx).GetHandle();
			depthAttachment.imageLayout = map.GetCurrentLayout();
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.clearValue.depthStencil = { 1.f, 0 };

			// -- Rendering Info --
			VkRenderingInfo renderingInfo{};
			renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			renderingInfo.renderArea.offset = { .x = 0, .y = 0 };
			renderingInfo.renderArea.extent = extent;
			renderingInfo.layerCount = 1;
			renderingInfo.pDepthAttachment = &depthAttachment;

			// -- Begin Rendering --
			vkCmdBeginRendering(vCmd, &renderingInfo);
			{
				// -- Set Dynamic Viewport --
				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = static_cast<float>(extent.width);
				viewport.height = static_cast<float>(extent.height);
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(vCmd, 0, 1, &viewport);

				// -- Set Dynamic Scissors --
				VkRect2D scissor{};
				scissor.offset = { .x = 0, .y = 0 };
				scissor.extent = extent;
				vkCmdSetScissor(vCmd, 0, 1, &scissor);

				// -- Bind Pipeline --
				vkCmdBindPipeline(vCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowPipeline.GetHandle());

				// -- Draw Models --
				for (const RenderItem& renderItem : renderItems)
				{
					Mesh* pMesh = renderItem.mesh;

					// -- Bind Model Data --
					pMesh->Bind(commandBuffer);

					// -- Draw Opaque --
					for (const SubMesh& subMesh : pMesh->vSubMeshes)
					{
						// -- Bind Push Constants --
						PushConstants pc
						{
							.lightSpace = lightItem.light->projMatrix * lightItem.light->viewMatrices[layerIdx-1],
							.model = renderItem.transform * subMesh.matrix
						};
						vkCmdPushConstants(vCmd, m_ShadowPipelineLayout.GetHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pc);

						// -- Drawing Time! --
						vkCmdDrawIndexed(vCmd, subMesh.indexCount, 1, subMesh.indexOffset, subMesh.vertexOffset, 0);
					}
				}
			}
			vkCmdEndRendering(vCmd);
		}


		// -- Ready outImage to be read from --
		map.TransitionLayout(commandBuffer,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			0, map.GetMipLevels(), 0, map.GetLayerCount());
	}
	Debugger::EndDebugLabel(commandBuffer);
}
