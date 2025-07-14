// -- Pompeii Includes --
#include "BlitPass.h"
#include "Context.h"
#include "Debugger.h"
#include "DescriptorPool.h"
#include "GeometryPass.h"
#include "Shader.h"
#include "Scene.h"
#include "Timer.h"

void pompeii::BlitPass::Initialize(const Context& context, const BlitPassCreateInfo& createInfo)
{
	// -- Descriptor Set Layout --
	{
		DescriptorSetLayoutBuilder builder{};
		builder
			.SetDebugName("Rendered HDR Texture | Camera Settings")
			.NewLayoutBinding()
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.NewLayoutBinding()
				.SetType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.NewLayoutBinding()
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(context, m_FragmentDSL);
		m_DeletionQueue.Push([&] { m_FragmentDSL.Destroy(context); });
		builder = {};
		builder
			.SetDebugName("HDR Image | Histogram | Average Luminance Last Frame")
			.NewLayoutBinding()
				.SetType(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
				.SetShaderStages(VK_SHADER_STAGE_COMPUTE_BIT)
			.NewLayoutBinding()
				.SetType(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
				.SetShaderStages(VK_SHADER_STAGE_COMPUTE_BIT)
			.NewLayoutBinding()
				.SetType(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
				.SetShaderStages(VK_SHADER_STAGE_COMPUTE_BIT)
			.Build(context, m_ComputeDSL);
		m_DeletionQueue.Push([&] { m_ComputeDSL.Destroy(context); });
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
			.AddLayout(m_ComputeDSL)
			.NewPushConstantRange()
				.SetPCSize(sizeof(glm::vec4))
				.SetPCStageFlags(VK_SHADER_STAGE_COMPUTE_BIT)
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
		ShaderModule compShader2;
		shaderLoader.Load(context, "shaders/generate_luminance_histogram.comp.spv", compShader);
		shaderLoader.Load(context, "shaders/get_average_luminance.comp.spv", compShader2);

		// Create pipelines
		ComputePipelineBuilder builder{};
		builder
			.SetDebugName("Compute Pipeline (Generate Luminance Histogram)")
			.SetPipelineLayout(m_ComputePipelineLayout)
			.SetShader(compShader)
			.Build(context, m_CompPipeHistogram);
		m_DeletionQueue.Push([&] { m_CompPipeHistogram.Destroy(context); });
		builder = {};
		builder
			.SetDebugName("Compute Pipeline (Average Luminance)")
			.SetPipelineLayout(m_ComputePipelineLayout)
			.SetShader(compShader2)
			.Build(context, m_CompPipeAverageLuminance);
		m_DeletionQueue.Push([&] { m_CompPipeAverageLuminance.Destroy(context); });

		compShader2.Destroy(context);
		compShader.Destroy(context);
	}

	// -- Sampler --
	{
		SamplerBuilder builder{};
		builder
			.SetFilters(VK_FILTER_NEAREST, VK_FILTER_NEAREST)
			.SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
			.SetMipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
			.SetMipLevels(0.f, 0.f, 0.f)
			.SetBorderColor(VK_BORDER_COLOR_INT_OPAQUE_BLACK)
			.Build(context, m_Sampler);
		m_DeletionQueue.Push([&] { m_Sampler.Destroy(context); });
	}

	// -- Buffers --
	{
		// Camera Settings
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

		// Histogram
		m_vHistogram.resize(createInfo.maxFramesInFlight);
		for (size_t i{}; i < createInfo.maxFramesInFlight; ++i)
		{
			BufferAllocator bufferAlloc{};
			bufferAlloc
				.SetDebugName("Histogram")
				.SetUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
				.SetSize(256 * sizeof(uint32_t))
				.Allocate(context, m_vHistogram[i]);
		}
		m_DeletionQueue.Push([&] { for (auto& ubo : m_vHistogram) ubo.Destroy(context); });
	}

	// -- Average Luminance Images --
	{
		m_vAverageLuminance.resize(createInfo.maxFramesInFlight);
		for (Image& image : m_vAverageLuminance)
		{
			ImageBuilder imageBuilder{};
			imageBuilder
				.SetDebugName("Average Luminance Target")
				.SetWidth(1)
				.SetHeight(1)
				.SetTiling(VK_IMAGE_TILING_OPTIMAL)
				.SetFormat(VK_FORMAT_R32_SFLOAT)
				.SetUsageFlags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT)
				.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				.Build(context, image);
			image.CreateView(context, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);
		}
		m_DeletionQueue.Push([&] { for (auto& img : m_vAverageLuminance) img.Destroy(context); });
	}

	// -- Descriptors --
	{
		m_vFragmentDS = context.descriptorPool->AllocateSets(context, m_FragmentDSL, createInfo.maxFramesInFlight, "Render Texture DS");
		m_vComputeLumDS = context.descriptorPool->AllocateSets(context, m_ComputeDSL, createInfo.maxFramesInFlight, "HDR Image | Average Luminance Last Frame | Histogram DS");
		m_vComputeAveDS = context.descriptorPool->AllocateSets(context, m_ComputeDSL, createInfo.maxFramesInFlight, "Average Luminance | Average Luminance Last Frame | Histogram DS");
		DescriptorSetWriter writer{};
		for (uint32_t i{}; i < createInfo.maxFramesInFlight; ++i)
		{
			uint32_t prevI = (i + createInfo.maxFramesInFlight - 1) % createInfo.maxFramesInFlight;
			// Fragment
			writer // HDR Image
				.AddImageInfo((*createInfo.renderImages)[i].GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_Sampler)
				.WriteImages(m_vFragmentDS[i], 0)
				.Execute(context);
			writer // Camera Settings
				.AddBufferInfo(m_vCameraSettings[i], 0, sizeof(ExposureSettings))
				.WriteBuffers(m_vFragmentDS[i], 1)
				.Execute(context);
			writer // Average Luminance
				.AddImageInfo(m_vAverageLuminance[i].GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_Sampler)
				.WriteImages(m_vFragmentDS[i], 2)
				.Execute(context);

			// Compute Luminance
			writer // HDR Image
				.AddImageInfo((*createInfo.renderImages)[i].GetView(), VK_IMAGE_LAYOUT_GENERAL)
				.WriteImages(m_vComputeLumDS[i], 0)
				.Execute(context);
			writer // Average Luminance Last Frame
				.AddImageInfo(m_vAverageLuminance[prevI].GetView(), VK_IMAGE_LAYOUT_GENERAL)
				.WriteImages(m_vComputeLumDS[i], 1)
				.Execute(context);
			writer // Histogram
				.AddBufferInfo(m_vHistogram[i], 0, 256 * sizeof(uint32_t))
				.WriteBuffers(m_vComputeLumDS[i], 2)
				.Execute(context);

			// Compute Average Luminance
			writer // HDR Image
				.AddImageInfo(m_vAverageLuminance[i].GetView(), VK_IMAGE_LAYOUT_GENERAL)
				.WriteImages(m_vComputeAveDS[i], 0)
				.Execute(context);
			writer // Average Luminance Last Frame
				.AddImageInfo(m_vAverageLuminance[prevI].GetView(), VK_IMAGE_LAYOUT_GENERAL)
				.WriteImages(m_vComputeAveDS[i], 1)
				.Execute(context);
			writer // Histogram
				.AddBufferInfo(m_vHistogram[i], 0, 256 * sizeof(uint32_t))
				.WriteBuffers(m_vComputeAveDS[i], 2)
				.Execute(context);
		}
	}
}

void pompeii::BlitPass::Destroy()
{
	m_DeletionQueue.Flush();
}

void pompeii::BlitPass::UpdateDescriptors(const Context& context, const std::vector<Image>& renderImages) const
{
	DescriptorSetWriter writer{};
	uint32_t count = static_cast<uint32_t>(renderImages.size());
	for (uint32_t i{}; i < count; ++i)
	{
		// Fragment
		writer // HDR Image
			.AddImageInfo(renderImages[i].GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_Sampler)
			.WriteImages(m_vFragmentDS[i], 0)
			.Execute(context);

		// Compute Luminance
		writer // HDR Image
			.AddImageInfo(renderImages[i].GetView(), VK_IMAGE_LAYOUT_GENERAL)
			.WriteImages(m_vComputeLumDS[i], 0)
			.Execute(context);
	}
}

void pompeii::BlitPass::RecordGraphic(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, const Image& renderImage, const Camera* pCamera)
{
	// -- Update Camera Settings --
	//todo really? every frame? camera exposure settings don't often change i feel ike, maybe this can be optimized using some dirty flag
	vmaCopyMemoryToAllocation(context.allocator, &pCamera->GetExposureSettings(), m_vCameraSettings[imageIndex].GetMemoryHandle(), 0, sizeof(ExposureSettings));

	m_vAverageLuminance[imageIndex].TransitionLayout(commandBuffer,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_2_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, 0, 1);

	// -- Set Up Attachment --
	VkRenderingAttachmentInfo colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView = renderImage.GetView().GetHandle();
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
	const VkCommandBuffer& vCmdBuffer = commandBuffer.GetHandle();

	// -- Render --
	Debugger::BeginDebugLabel(commandBuffer, "Tone Mapping | Exposure Pass", glm::vec4(0.6f, 0.2f, 0.8f, 1));
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
void pompeii::BlitPass::RecordCompute(const Context&, CommandBuffer& commandBuffer, uint32_t imageIndex, const Image& renderImage)
{
	// -- Compute --
	Debugger::BeginDebugLabel(commandBuffer, "Compute Luminance | Exposure Pass", glm::vec4(0.6f, 0.2f, 0.8f, 1));
	{
		// -- Bind First Pipeline --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Pipeline (Compute | Luminance Histogram)", glm::vec4(0.2f, 0.4f, 1.f, 1.f));
		vkCmdBindPipeline(commandBuffer.GetHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_CompPipeHistogram.GetHandle());
		Debugger::InsertDebugLabel(commandBuffer, "Bind Luminance Histogram Descriptor", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(commandBuffer.GetHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipelineLayout.GetHandle(), 0, 1, &m_vComputeLumDS[imageIndex].GetHandle(), 0, nullptr);

		// -- Bind Push Constants --
		//todo hardcoded min and range? eeh??
		const VkExtent2D extent = renderImage.GetExtent2D();
		glm::vec4 param =
		{
			-8.f,							// minLogLum
			12.f,							// logLumRange
			Timer::GetDeltaSeconds(),		// deltaS
			extent.width* extent.height		// numPixels
		};
		vkCmdPushConstants(commandBuffer.GetHandle(), m_ComputePipelineLayout.GetHandle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(param), &param);

		// -- Compute 1 --
		const uint32_t groupCountX = (extent.width + 15) / 16;
		const uint32_t groupCountY = (extent.height + 15) / 16;
		vkCmdDispatch(commandBuffer.GetHandle(), groupCountX, groupCountY, 1);

		// -- Memory Barriers --
		m_vAverageLuminance[imageIndex].TransitionLayout(commandBuffer,
			VK_IMAGE_LAYOUT_GENERAL,
			VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
			VK_ACCESS_2_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, 0, 1);
		m_vAverageLuminance[(imageIndex + m_vAverageLuminance.size() - 1) % m_vAverageLuminance.size()].TransitionLayout(commandBuffer,
			VK_IMAGE_LAYOUT_GENERAL,
			VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
			VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, 0, 1);
		m_vHistogram[imageIndex].InsertBarrier(commandBuffer,
			VK_ACCESS_2_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
			VK_ACCESS_2_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);

		// -- Bind Second Pipeline --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Pipeline (Compute | Average Luminance)", glm::vec4(0.2f, 0.4f, 1.f, 1.f));
		vkCmdBindPipeline(commandBuffer.GetHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_CompPipeAverageLuminance.GetHandle());
		Debugger::InsertDebugLabel(commandBuffer, "Bind Average Luminance Histogram Descriptor", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(commandBuffer.GetHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipelineLayout.GetHandle(), 0, 1, &m_vComputeAveDS[imageIndex].GetHandle(), 0, nullptr);

		// -- Compute 2 --
		vkCmdDispatch(commandBuffer.GetHandle(), 1, 1, 1);
	}
	Debugger::EndDebugLabel(commandBuffer);
}
