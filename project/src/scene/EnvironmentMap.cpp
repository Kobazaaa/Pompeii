// -- Standard Library --
#include <array>

// -- Pompeii Includes --
#include "EnvironmentMap.h"
#include "Context.h"
#include "DescriptorSet.h"
#include "Pipeline.h"
#include "Shader.h"
#include "Debugger.h"
#include "Material.h"

// -- Math Includes --
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::EnvironmentMap::Destroy(const Context& context)
{
	m_DiffuseIrradiance.Destroy(context);
	m_Skybox.Destroy(context);
	m_Sampler.Destroy(context);
}

//--------------------------------------------------
//    Initializers
//--------------------------------------------------
pom::EnvironmentMap& pom::EnvironmentMap::CreateSampler(const Context& context)
{
	SamplerBuilder builder{};
	builder
		.SetFilters(VK_FILTER_LINEAR, VK_FILTER_LINEAR)
		.SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
		.SetMipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
		.SetMipLevels(0.f, 0.f, VK_LOD_CLAMP_NONE)
		.SetBorderColor(VK_BORDER_COLOR_INT_OPAQUE_BLACK)
		.Build(context, m_Sampler);
	return *this;
}
pom::EnvironmentMap& pom::EnvironmentMap::CreateSkyboxCube(const Context& context, const std::string& path, uint32_t size)
{
	// -- Load HDRI Texture on CPU --
	Texture tex{ path, VK_FORMAT_R32G32B32A32_SFLOAT, true};
	glm::ivec2 extent = tex.GetExtent();

	// -- Build HDR Image on GPU --
	Image HDRI{};
	ImageBuilder builder{};
	builder
		.SetDebugName(tex.GetPath().c_str())
		.SetWidth(extent.x)
		.SetHeight(extent.y)
		.SetFormat(tex.GetFormat())
		.SetTiling(VK_IMAGE_TILING_OPTIMAL)
		.SetUsageFlags(VK_IMAGE_USAGE_SAMPLED_BIT)
		.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		.InitialData(tex.GetPixels(), 0, extent.x, extent.y, tex.GetMemorySize(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		.Build(context, HDRI);
	HDRI.CreateView(context, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);

	// -- Build Cube Map Image on GPU --
	uint32_t maxMipsLevels = static_cast<uint32_t>(std::floor(std::log2(size))) + 1;
	builder = {};
	builder
		.SetDebugName("Cube Map Skybox")
		.SetWidth(size)
		.SetHeight(size)
		.SetFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
		.SetTiling(VK_IMAGE_TILING_OPTIMAL)
		.SetMipLevels(maxMipsLevels)
		.SetUsageFlags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
		.SetCreateFlags(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
		.SetArrayLayers(6) // 6 beautiful cubic faces :)
		.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		.Build(context, m_Skybox);
	// -- Generate the 6 views to each face --
	std::array<std::vector<ImageView>, 6> faces{};
	for (uint32_t i{}; i < 6; ++i)
	{
		faces[i].resize(1);
		faces[i][0] = m_Skybox.CreateView(context, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, i, 1);
	}

	// -- Render To CubeMap --
	RenderToCubeMap(context, "shaders/cubemap.vert.spv", "shaders/cubemap.frag.spv", 
				 HDRI, HDRI.GetView(), m_Sampler, 
				 m_Skybox, faces, size);
	m_Skybox.GenerateMipMaps(context, size, size, maxMipsLevels, 6, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	m_Skybox.DestroyAllViews(context);

	// -- Generate a view to all faces --
	m_Skybox.CreateView(context, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_CUBE, 0, m_Skybox.GetMipLevels(), 0, 6);

	HDRI.Destroy(context);
	return *this;
}

pom::EnvironmentMap& pom::EnvironmentMap::CreateDiffIrradianceMap(const Context& context, uint32_t size)
{
	assert(m_Skybox.GetHandle() && "Cannot create a diffuse irradiance map without the skybox being set up!");
	
	// -- Build Cube Map Image on GPU --
	ImageBuilder builder{};
	builder
		.SetDebugName("Cube Map Diffuse Irradiance")
		.SetWidth(size)
		.SetHeight(size)
		.SetFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
		.SetTiling(VK_IMAGE_TILING_OPTIMAL)
		.SetUsageFlags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
		.SetCreateFlags(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
		.SetArrayLayers(6) // 6 beautiful cubic faces :)
		.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		.Build(context, m_DiffuseIrradiance);
	// -- Generate the 6 views to each face --
	std::array<std::vector<ImageView>, 6> faces{};
	for (uint32_t i{}; i < 6; ++i)
	{
		faces[i].resize(1);
		faces[i][0] = m_DiffuseIrradiance.CreateView(context, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, i, 1);
	}

	// -- Render To CubeMap --
	RenderToCubeMap(context, "shaders/cubemap.vert.spv", "shaders/diffuse_irradiance.frag.spv",
		m_Skybox, m_Skybox.GetView(), m_Sampler,
		m_DiffuseIrradiance, faces, size);
	m_DiffuseIrradiance.DestroyAllViews(context);

	// -- Generate a view to all faces --
	m_DiffuseIrradiance.CreateView(context, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_CUBE, 0, 1, 0, 6);

	return *this;
}

//--------------------------------------------------
//    Accessors
//--------------------------------------------------
const pom::Sampler& pom::EnvironmentMap::GetSampler()				const { return m_Sampler; }
const pom::Image& pom::EnvironmentMap::GetSkybox()					const { return m_Skybox; }
const pom::Image& pom::EnvironmentMap::GetDiffuseIrradianceMap()	const { return m_DiffuseIrradiance; }

//--------------------------------------------------
//    Helpers
//--------------------------------------------------
void pom::EnvironmentMap::RenderToCubeMap(const Context& context, const std::string& vert, const std::string& frag,
										  Image&, const ImageView& inView, const Sampler& inSampler,
										  Image& outImage, std::array<std::vector<ImageView>, 6>& outViews, uint32_t size)
{
	// -- Push Constant Struct --
	struct PC
	{
		glm::mat4 view;
		glm::mat4 projection;
	};

	// -- Descriptor Set Layout --
	DescriptorSetLayout DSL{};
	DescriptorSetLayoutBuilder DSLBuilder{};
	DSLBuilder
		.SetDebugName("Render To CubeMap DSL")
		.NewLayoutBinding()
			.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
		.Build(context, DSL);

	// -- Pipeline Layout --
	PipelineLayout pipelineLayout{};
	PipelineLayoutBuilder pipelineLayoutBuilder{};
	pipelineLayoutBuilder
		.NewPushConstantRange()
			.SetPCSize(sizeof(PC))
			.SetPCStageFlags(VK_SHADER_STAGE_VERTEX_BIT)
		.AddLayout(DSL)
		.Build(context, pipelineLayout);

	// -- Load Shaders --
	ShaderLoader shaderLoader{};
	ShaderModule vertShader;
	ShaderModule fragShader;
	shaderLoader.Load(context, vert, vertShader);
	shaderLoader.Load(context, frag, fragShader);

	// -- Pipeline --
	VkPipelineRenderingCreateInfo renderingCreateInfo{};
	VkFormat format = outImage.GetFormat();
	renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingCreateInfo.colorAttachmentCount = 1;
	renderingCreateInfo.pColorAttachmentFormats = &format;

	Pipeline pipeline{};
	GraphicsPipelineBuilder pipelineBuilder{};
	pipelineBuilder
		.SetDebugName("Graphics Pipeline (Render To CubeMap)")
		.SetPipelineLayout(pipelineLayout)
		.SetupDynamicRendering(renderingCreateInfo)
		.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
		.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
		.AddShader(vertShader, VK_SHADER_STAGE_VERTEX_BIT)
		.AddShader(fragShader, VK_SHADER_STAGE_FRAGMENT_BIT)
		.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
		.SetCullMode(VK_CULL_MODE_NONE)
		.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
		.SetPolygonMode(VK_POLYGON_MODE_FILL)
		.SetDepthTest(VK_FALSE, VK_FALSE, VK_COMPARE_OP_NEVER)
		.Build(context, pipeline);

	// -- Allocate & Write Descriptor Set --
	DescriptorSet DS{};
	DS = context.descriptorPool->AllocateSets(context, DSL, 1, "Render To CubeMap DS").front();
	DescriptorSetWriter writer{};
	writer
		.AddImageInfo(inView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, inSampler)
		.WriteImages(DS, 0)
		.Execute(context);

	// -- CPU Views per Face --
	glm::vec3 eye = glm::vec3(0.f);
	glm::mat4 captureViews[6] = {
		glm::lookAt(eye, eye + glm::vec3( 1.f,  0.f,  0.f), glm::vec3(0.f, -1.f,  0.f)), // +X
		glm::lookAt(eye, eye + glm::vec3(-1.f,  0.f,  0.f), glm::vec3(0.f, -1.f,  0.f)), // -X
		glm::lookAt(eye, eye + glm::vec3( 0.f, -1.f,  0.f), glm::vec3(0.f,  0.f, -1.f)), // -Y
		glm::lookAt(eye, eye + glm::vec3( 0.f,  1.f,  0.f), glm::vec3(0.f,  0.f,  1.f)), // +Y
		glm::lookAt(eye, eye + glm::vec3( 0.f,  0.f,  1.f), glm::vec3(0.f, -1.f,  0.f)), // +Z
		glm::lookAt(eye, eye + glm::vec3( 0.f,  0.f, -1.f), glm::vec3(0.f, -1.f,  0.f)), // -Z
	};
	glm::mat4 captureProj = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
	captureProj[1][1] *= -1.f;

	// -- Render --
	CommandBuffer& cmd = context.commandPool->AllocateCmdBuffers(1);
	cmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	{
		Debugger::BeginDebugLabel(cmd, "Render To CubeMap", glm::vec4(0.6f, 0.2f, 0.8f, 1));
		VkCommandBuffer vCmd = cmd.GetHandle();

		// -- Ready outImage to be rendered to --
		outImage.TransitionLayout(cmd,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, outImage.GetMipLevels(), 0, outImage.GetLayerCount());

		// -- Render --
		for (uint32_t layerIdx{}; layerIdx < outViews.size(); ++layerIdx)
		{
			for (uint32_t mipIdx{}; mipIdx < outViews[layerIdx].size(); ++mipIdx)
			{
				// -- Setup Attachment --
				VkRenderingAttachmentInfo colorAttachment{};
				colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				colorAttachment.imageView = outViews[layerIdx][mipIdx].GetHandle();
				colorAttachment.imageLayout = outImage.GetCurrentLayout();
				colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				colorAttachment.clearValue.color = { {0.f, 0.f, 0.f, 1.0f} };

				// -- Rendering Info --
				VkRenderingInfo renderingInfo{};
				renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
				renderingInfo.renderArea.offset = {.x = 0, .y = 0};
				renderingInfo.renderArea.extent = {.width = size, .height = size};
				renderingInfo.layerCount = 1;
				renderingInfo.colorAttachmentCount = 1;
				renderingInfo.pColorAttachments = &colorAttachment;

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

					// -- Bind Descriptors --
					vkCmdBindDescriptorSets(vCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.GetHandle(), 0, 1, &DS.GetHandle(), 0, nullptr);

					// -- Push Constants --
					PC pc
					{
						.view = captureViews[layerIdx],
						.projection = captureProj
					};
					vkCmdPushConstants(vCmd, pipelineLayout.GetHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PC), &pc);

					// -- Draw --
					vkCmdDraw(vCmd, 36, 1, 0, 0);
				}
				vkCmdEndRendering(vCmd);
			}
		}

		// -- Ready outImage to be read from --
		outImage.TransitionLayout(cmd,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			0, outImage.GetMipLevels(), 0, outImage.GetLayerCount());

		Debugger::EndDebugLabel(cmd);
	}
	cmd.End();
	cmd.Submit(context.device.GetGraphicQueue(), true);
	cmd.Free(context.device);

	// -- Cleanup --
	pipeline.Destroy(context);
	fragShader.Destroy(context);
	vertShader.Destroy(context);
	pipelineLayout.Destroy(context);
	DSL.Destroy(context);
}
