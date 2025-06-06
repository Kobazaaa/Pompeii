// _- Math Includes --
#include <glm/gtc/matrix_transform.hpp>

// -- Pompeii Includes --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "Light.h"
#include "Model.h"
#include "Context.h"
#include "Shader.h"
#include "Pipeline.h"
#include "Debugger.h"
#include "Scene.h"

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::Light::Light(const glm::vec3& dirPos, const glm::vec3& col, float luxLumen, Type type)
	: m_Type(type)
	, dirPos(type == Type::Directional ? glm::normalize(dirPos) : dirPos)
	, color(col)
	, luxLumen(luxLumen)
{ }
void pom::Light::DestroyDepthMap(const Context& context)
{
	m_DepthMap.Destroy(context);
}

pom::Light::Light(Light&& other) noexcept
{
	m_Type = other.m_Type;
	dirPos = std::move(other.dirPos);
	color = std::move(other.color);
	luxLumen = other.luxLumen;
	viewMatrices = std::move(other.viewMatrices);
	other.viewMatrices.clear();
	projMatrix = std::move(other.projMatrix);
	m_DepthMap = std::move(other.m_DepthMap);
}
pom::Light& pom::Light::operator=(Light&& other) noexcept
{
	if (this == &other)
		return *this;
	m_Type = other.m_Type;
	dirPos = std::move(other.dirPos);
	color = std::move(other.color);
	luxLumen = other.luxLumen;
	viewMatrices = std::move(other.viewMatrices);
	other.viewMatrices.clear();
	projMatrix = std::move(other.projMatrix);
	m_DepthMap = std::move(other.m_DepthMap);
	return *this;
}

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
pom::Light::Type pom::Light::GetType()		const { return m_Type; }
const pom::Image& pom::Light::GetDepthMap()	const { return m_DepthMap; }

void pom::Light::GenerateDepthMap(const Context& context, const Scene* pScene, uint32_t size)
{
	if (m_DepthMap.GetHandle() != VK_NULL_HANDLE)
		m_DepthMap.Destroy(context);

	// -- Build Depth Map Image on GPU --
	ImageBuilder builder{};
	builder
		.SetDebugName("Light Depth Map")
		.SetWidth(size)
		.SetHeight(size)
		.SetFormat(VK_FORMAT_D32_SFLOAT)
		.SetTiling(VK_IMAGE_TILING_OPTIMAL)
		.SetUsageFlags(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
		.SetCreateFlags(m_Type == Type::Point ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0)
		.SetArrayLayers(m_Type == Type::Point ? 6 : 1)
		.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		.Build(context, m_DepthMap);

	// -- Generate the 6 views to each face --
	std::vector<ImageView> faces{};
	faces.resize(m_Type == Type::Point ? 6 : 1);
	for (uint32_t i{}; i < faces.size(); ++i)
		faces[i] = m_DepthMap.CreateView(context, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, i, 1);

	// -- Render To CubeMap --
	GenerateDepthMap(context, pScene, m_DepthMap, faces, size);
	m_DepthMap.DestroyAllViews(context);

	// -- Generate a view to all faces --
	m_DepthMap.CreateView(context, VK_IMAGE_ASPECT_DEPTH_BIT, m_Type == Type::Point ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D, 0, m_DepthMap.GetMipLevels(), 0, m_DepthMap.GetLayerCount());
}
void pom::Light::GenerateDepthMap(const Context& context, const Scene* pScene, Image& outImage, std::vector<ImageView>& outViews, uint32_t size)
{
	// -- Push Constant Struct --
	struct PC
	{
		glm::mat4 projview;
		glm::mat4 model;
	};

	// -- Pipeline Layout --
	PipelineLayout pipelineLayout{};
	PipelineLayoutBuilder pipelineLayoutBuilder{};
	pipelineLayoutBuilder
		.NewPushConstantRange()
		.SetPCSize(sizeof(PC))
		.SetPCStageFlags(VK_SHADER_STAGE_VERTEX_BIT)
		.Build(context, pipelineLayout);

	// -- Load Shaders --
	ShaderLoader shaderLoader{};
	ShaderModule vertShader;
	//ShaderModule fragShader;
	shaderLoader.Load(context, "shaders/shadowmap.vert.spv", vertShader);
	//shaderLoader.Load(context, "frag", fragShader);

	// -- Pipeline --
	VkPipelineRenderingCreateInfo renderingCreateInfo{};
	VkFormat format = outImage.GetFormat();
	renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingCreateInfo.depthAttachmentFormat = format;

	Pipeline pipeline{};
	GraphicsPipelineBuilder pipelineBuilder{};
	pipelineBuilder
		.SetDebugName("Graphics Pipeline (Generate Light Depth Map)")
		.SetPipelineLayout(pipelineLayout)
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
		.Build(context, pipeline);

	// -- Render --
	CommandBuffer& cmd = context.commandPool->AllocateCmdBuffers(1);
	cmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	{
		Debugger::BeginDebugLabel(cmd, "Render Light DepthMap", glm::vec4(0.6f, 0.2f, 0.8f, 1));
		const VkCommandBuffer& vCmd = cmd.GetHandle();

		// -- Ready outImage to be rendered to --
		outImage.TransitionLayout(cmd,
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
			0, outImage.GetMipLevels(), 0, outImage.GetLayerCount());

		// -- Render --
		for (uint32_t layerIdx{}; layerIdx < outViews.size(); ++layerIdx)
		{
			// -- Setup Attachment --
			VkRenderingAttachmentInfo depthAttachment{};
			depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depthAttachment.imageView = outViews[layerIdx].GetHandle();
			depthAttachment.imageLayout = outImage.GetCurrentLayout();
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.clearValue.depthStencil = { 1.f, 0 };

			// -- Rendering Info --
			VkRenderingInfo renderingInfo{};
			renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			renderingInfo.renderArea.offset = { .x = 0, .y = 0 };
			renderingInfo.renderArea.extent = { .width = size, .height = size };
			renderingInfo.layerCount = 1;
			renderingInfo.pDepthAttachment = &depthAttachment;

			// -- Begin Rendering --
			vkCmdBeginRendering(vCmd, &renderingInfo);
			{
				// -- Set Dynamic Viewport --
				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = static_cast<float>(size);
				viewport.height = static_cast<float>(size);
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(vCmd, 0, 1, &viewport);

				// -- Set Dynamic Scissors --
				VkRect2D scissor{};
				scissor.offset = { .x = 0, .y = 0 };
				scissor.extent = { .width = size, .height = size };
				vkCmdSetScissor(vCmd, 0, 1, &scissor);

				// -- Bind Pipeline --
				vkCmdBindPipeline(vCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetHandle());

				// -- Draw Models --
				for (const Model& model : pScene->GetModels())
				{
					// -- Bind Model Data --
					model.Bind(cmd);

					// -- Draw Opaque --
					for (const Mesh& mesh : model.opaqueMeshes)
					{
						// -- Bind Push Constants --
						PC pc
						{
							.projview = projMatrix * viewMatrices[layerIdx],
							.model = mesh.matrix
						};
						vkCmdPushConstants(vCmd, pipelineLayout.GetHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PC), &pc);

						// -- Drawing Time! --
						vkCmdDrawIndexed(vCmd, mesh.indexCount, 1, mesh.indexOffset, mesh.vertexOffset, 0);
					}

					// -- Draw Transparent using Alpha Cut-Off --
					for (const Mesh& mesh : model.transparentMeshes)
					{
						// -- Bind Push Constants --
						PC pc
						{
							.projview = projMatrix * viewMatrices[layerIdx],
							.model = mesh.matrix
						};
						vkCmdPushConstants(vCmd, pipelineLayout.GetHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PC), &pc);

						// -- Drawing Time! --
						vkCmdDrawIndexed(vCmd, mesh.indexCount, 1, mesh.indexOffset, mesh.vertexOffset, 0);
					}
				}
			}
			vkCmdEndRendering(vCmd);
		}

		// -- Ready outImage to be read from --
		outImage.TransitionLayout(cmd,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			0, outImage.GetMipLevels(), 0, outImage.GetLayerCount());

		Debugger::EndDebugLabel(cmd);
	}
	cmd.End();
	cmd.Submit(context.device.GetGraphicQueue(), true);
	cmd.Free(context.device);

	// -- Cleanup --
	pipeline.Destroy(context);
	//fragShader.Destroy(context);
	vertShader.Destroy(context);
	pipelineLayout.Destroy(context);
}
