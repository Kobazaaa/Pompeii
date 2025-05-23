// -- Pompeii Includes --
#include "BlitPass.h"
#include "Context.h"
#include "Debugger.h"
#include "DescriptorPool.h"
#include "GeometryPass.h"
#include "Shader.h"
#include "Scene.h"

void pom::BlitPass::Initialize(const Context& context, const BlitPassCreateInfo& createInfo)
{
	// -- Descriptor Set Layout --
	{
		DescriptorSetLayoutBuilder builder{};
		builder
			.SetDebugName("Rendered Texture | Camera Settings")
			.NewLayoutBinding()
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.NewLayoutBinding()
				.SetType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(context, m_FragmentDSL);
		m_DeletionQueue.Push([&] { m_FragmentDSL.Destroy(context); });
	}

	// -- Pipeline Layout --
	{
		PipelineLayoutBuilder builder{};
		builder
			.AddLayout(m_FragmentDSL)
			.Build(context, m_PipelineLayout);
		m_DeletionQueue.Push([&] {m_PipelineLayout.Destroy(context); });

		builder = {};
		builder
			//.AddLayout(m_FragmentDSL)
			.Build(context, m_ComputePipelineLayout);
		m_DeletionQueue.Push([&] { m_ComputePipelineLayout.Destroy(context); });
	}

	// -- Graphics Pipeline --
	{
		// Load in shaders
		ShaderLoader shaderLoader{};
		ShaderModule vertShader;
		ShaderModule fragShader;
		shaderLoader.Load(context, "shaders/fullscreenTri.vert.spv", vertShader);
		shaderLoader.Load(context, "shaders/blit.frag.spv", fragShader);

		// Setup dynamic rendering info
		VkPipelineRenderingCreateInfo renderingCreateInfo{};
		VkFormat format = createInfo.format;
		renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingCreateInfo.colorAttachmentCount = 1;
		renderingCreateInfo.pColorAttachmentFormats = &format;

		// Create pipeline
		GraphicsPipelineBuilder builder{};
		builder
			.SetDebugName("Graphics Pipeline (Blitting)")
			.SetPipelineLayout(m_PipelineLayout)
			.SetupDynamicRendering(renderingCreateInfo)
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.AddShader(vertShader, VK_SHADER_STAGE_VERTEX_BIT)
			.AddShader(fragShader, VK_SHADER_STAGE_FRAGMENT_BIT)
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetCullMode(VK_CULL_MODE_BACK_BIT)
			.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.SetPolygonMode(VK_POLYGON_MODE_FILL)
			.SetDepthTest(VK_FALSE, VK_FALSE, VK_COMPARE_OP_NEVER)
			.Build(context, m_Pipeline);
		m_DeletionQueue.Push([&] { m_Pipeline.Destroy(context); });

		fragShader.Destroy(context);
		vertShader.Destroy(context);
	}

	// -- Compute Pipeline --
	{
		// Load in shaders
		ShaderLoader shaderLoader{};
		ShaderModule compShader;
		shaderLoader.Load(context, "shaders/autoexposure.comp.spv", compShader);

		// Create pipeline
		ComputePipelineBuilder builder{};
		builder
			.SetDebugName("Compute Pipeline (Auto Exposure)")
			.SetPipelineLayout(m_ComputePipelineLayout)
			.SetShader(compShader)
			.Build(context, m_ComputePipeline);
		m_DeletionQueue.Push([&] { m_ComputePipeline.Destroy(context); });

		compShader.Destroy(context);
	}

	// -- Sampler --
	{
		SamplerBuilder builder{};
		builder
			.SetFilters(VK_FILTER_NEAREST, VK_FILTER_NEAREST)
			.SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
			.SetMipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
			.SetBorderColor(VK_BORDER_COLOR_INT_OPAQUE_BLACK)
			.Build(context, m_Sampler);
		m_DeletionQueue.Push([&] { m_Sampler.Destroy(context); });
	}

	// -- Buffers --
	{
		m_vCameraSettings.resize(createInfo.maxFramesInFlight);
		for (size_t i{}; i < createInfo.maxFramesInFlight; ++i)
		{
			BufferAllocator bufferAlloc{};
			bufferAlloc
				.SetDebugName("Camera Settings UBO")
				.SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
				.SetSize(sizeof(ExposureSettings))
				.HostAccess(true)
				.Allocate(context, m_vCameraSettings[i]);
		}
		m_DeletionQueue.Push([&] { for (auto& ubo : m_vCameraSettings) ubo.Destroy(context); });
	}

	// -- Descriptors --
	{
		m_vFragmentDS = createInfo.pDescriptorPool->AllocateSets(context, m_FragmentDSL, createInfo.maxFramesInFlight, "Render Texture DS");
		DescriptorSetWriter writer{};
		for (uint32_t i{}; i < m_vFragmentDS.size(); ++i)
		{
			writer
				.AddImageInfo((*createInfo.renderImages)[i],
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_Sampler)
				.WriteImages(m_vFragmentDS[i], 0)
				.Execute(context);
			writer
				.AddBufferInfo(m_vCameraSettings[i], 0, sizeof(ExposureSettings))
				.WriteBuffers(m_vFragmentDS[i], 1)
				.Execute(context);
		}
	}
}

void pom::BlitPass::Destroy()
{
	m_DeletionQueue.Flush();
}

void pom::BlitPass::UpdateDescriptors(const Context& context, const std::vector<Image>& renderImages) const
{
	DescriptorSetWriter writer{};
	for (uint32_t i{}; i < m_vFragmentDS.size(); ++i)
	{
		writer
			.AddImageInfo(renderImages[i],
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_Sampler)
			.WriteImages(m_vFragmentDS[i], 0)
			.Execute(context);
	}
}

void pom::BlitPass::Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, const Image& renderImage, const Camera* pCamera) const
{
	// -- Update Camera Settings --
	//todo really? every frame? camera exposure settings don't often change i feel ike, maybe this can be optimized using some dirty flag
	vmaCopyMemoryToAllocation(context.allocator, &pCamera->GetExposureSettings(), m_vCameraSettings[imageIndex].GetMemoryHandle(), 0, sizeof(ExposureSettings));

	// -- Set Up Attachment --
	VkRenderingAttachmentInfo colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView = renderImage.GetViewHandle();
	colorAttachment.imageLayout = renderImage.GetCurrentLayout();
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.clearValue.color = { {0.f, 0.f, 0.f, 1.0f} };

	// -- Render Info --
	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea = VkRect2D{ VkOffset2D{0, 0}, renderImage.GetExtent2D() };
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;

	// -- Render --
	const VkCommandBuffer& vCmdBuffer = commandBuffer.GetHandle();
	Debugger::BeginDebugLabel(commandBuffer, "Blitting Pass", glm::vec4(0.6f, 0.2f, 0.8f, 1));
	vkCmdBeginRendering(vCmdBuffer, &renderingInfo);
	{
		// -- Set Dynamic Viewport --
		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(renderImage.GetExtent2D().width);
		viewport.height = static_cast<float>(renderImage.GetExtent2D().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		Debugger::InsertDebugLabel(commandBuffer, "Bind Viewport", glm::vec4(0.2f, 1.f, 0.2f, 1.f));
		vkCmdSetViewport(vCmdBuffer, 0, 1, &viewport);

		// -- Set Dynamic Scissors --
		VkRect2D scissor;
		scissor.offset = { .x = 0, .y = 0 };
		scissor.extent = renderImage.GetExtent2D();
		Debugger::InsertDebugLabel(commandBuffer, "Bind Scissor", glm::vec4(1.f, 1.f, 0.2f, 1.f));
		vkCmdSetScissor(vCmdBuffer, 0, 1, &scissor);

		// -- Bind Descriptor Sets --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Rendered Image | Camera Settings", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout.GetHandle(), 0, 1, &m_vFragmentDS[imageIndex].GetHandle(), 0, nullptr);

		// -- Draw Triangle --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Pipeline (Blitting)", glm::vec4(0.2f, 0.4f, 1.f, 1.f));
		vkCmdBindPipeline(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetHandle());
		Debugger::InsertDebugLabel(commandBuffer, "Draw Full Screen Triangle", glm::vec4(0.4f, 0.8f, 1.f, 1.f));
		vkCmdDraw(commandBuffer.GetHandle(), 3, 1, 0, 0);
	}
	vkCmdEndRendering(vCmdBuffer);
	Debugger::EndDebugLabel(commandBuffer);
}
