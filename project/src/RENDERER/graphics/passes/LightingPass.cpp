// -- Pompeii Includes --
#include "LightingPass.h"
#include "EnvironmentMap.h"
#include "Context.h"
#include "Debugger.h"
#include "GBuffer.h"
#include "GeometryPass.h"
#include "Shader.h"
#include "GPUCamera.h"
#include "RenderingItems.h"

void pompeii::LightingPass::Initialize(const Context& context, const LightingPassCreateInfo& createInfo)
{
	// -- Descriptor Set Layout --
	{
		// Camera Matrices
		DescriptorSetLayoutBuilder builder{};
		builder
			.SetDebugName("Cam Layout")
			.NewLayoutBinding()
			.SetType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(context, m_CameraMatricesDSL);
		m_DeletionQueue.Push([&] { m_CameraMatricesDSL.Destroy(context); });

		// Light SSBO
		builder = {};
		builder
			.SetDebugName("Light Layout")
			.NewLayoutBinding()
			.SetType(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
			.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(context, m_SSBOLightDSL);
		m_DeletionQueue.Push([&] { m_SSBOLightDSL.Destroy(context); });

		// Light UBO Depth Images
		builder = {};
		builder
			.SetDebugName("Light Depth Maps Layout")
			.NewLayoutBinding()
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
				.SetCount(256)
				.AddBindingFlags(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT)
				.AddBindingFlags(VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)
				.AddBindingFlags(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
			.AddLayoutFlag(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
			.Build(context, m_UBOLightMapDSL);
		m_DeletionQueue.Push([&] { m_UBOLightMapDSL.Destroy(context); });

		// GBuffer Images
		builder = {};
		builder
			.SetDebugName("GBuffer Textures Layout")
			.NewLayoutBinding() // Albedo
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.NewLayoutBinding() // Normal
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.NewLayoutBinding() // WorldPos
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.NewLayoutBinding() // Roughness Metallic
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.NewLayoutBinding() // Depth
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.NewLayoutBinding() // Environment Map
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.NewLayoutBinding() // Diffuse Irradiance
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.NewLayoutBinding() // Specular Irradiance
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.NewLayoutBinding() // BRDF LUT
				.SetType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.SetShaderStages(VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(context, m_GBufferTexturesDSL);
		m_DeletionQueue.Push([&] { m_GBufferTexturesDSL.Destroy(context); });
	}

	// -- Pipeline Layout --
	{
		PipelineLayoutBuilder builder{};

		builder
			.AddLayout(m_CameraMatricesDSL)
			.AddLayout(m_SSBOLightDSL)
			.AddLayout(m_UBOLightMapDSL)
			.AddLayout(m_UBOLightMapDSL)
			.AddLayout(m_GBufferTexturesDSL)
			.Build(context, m_PipelineLayout);
		m_DeletionQueue.Push([&] {m_PipelineLayout.Destroy(context); });
	}

	// -- Pipelines --
	{
		// Load in shaders
		ShaderLoader shaderLoader{};
		ShaderModule vertShader;
		ShaderModule fragShader;
		shaderLoader.Load(context, "shaders/fullscreenTri.vert.spv", vertShader);
		shaderLoader.Load(context, "shaders/lighting.frag.spv", fragShader);

		// Setup dynamic rendering info
		VkPipelineRenderingCreateInfo renderingCreateInfo{};
		VkFormat format = createInfo.format;
		renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingCreateInfo.colorAttachmentCount = 1;
		renderingCreateInfo.pColorAttachmentFormats = &format;

		// Create pipeline
		GraphicsPipelineBuilder builder{};
		builder
			.SetDebugName("Graphics Pipeline (Lighting)")
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

	// -- Sampler --
	{
		SamplerBuilder builder{};
		builder
			.SetFilters(VK_FILTER_NEAREST, VK_FILTER_NEAREST)
			.SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
			.SetMipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
			.SetMipLevels(0.f, 0.f, 0.f)
			.SetBorderColor(VK_BORDER_COLOR_INT_OPAQUE_BLACK)
			.Build(context, m_GBufferSampler);
		m_DeletionQueue.Push([&] { m_GBufferSampler.Destroy(context); });
		builder = {};
		builder
			.SetFilters(VK_FILTER_LINEAR, VK_FILTER_LINEAR) // pcf
			.SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
			.SetMipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
			.SetMipLevels(0.f, 0.f, 0.f)
			.SetBorderColor(VK_BORDER_COLOR_INT_OPAQUE_BLACK)
			.EnableCompare(VK_COMPARE_OP_LESS)
			.Build(context, m_ShadowSampler);
		m_DeletionQueue.Push([&] { m_ShadowSampler.Destroy(context); });
	}

	// -- UBO --
	{
		m_vCameraMatrices.resize(context.maxFramesInFlight);
		for (size_t i{}; i < context.maxFramesInFlight; ++i)
		{
			BufferAllocator bufferAlloc{};
			bufferAlloc
				.SetDebugName("Cam UBO")
				.SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
				.SetSize(sizeof(glm::mat4) * 2)
				.HostAccess(true)
				.Allocate(context, m_vCameraMatrices[i]);
		}
		m_DeletionQueue.Push([&] { for (auto& ubo : m_vCameraMatrices) ubo.Destroy(context); });
	}

	// -- Buffers --
	{
		m_vUBODirLightMapDS.resize(context.maxFramesInFlight);
		m_vUBOPointLightMapDS.resize(context.maxFramesInFlight);

		m_vGBufferTexturesDS = context.descriptorPool->AllocateSets(context, m_GBufferTexturesDSL, context.maxFramesInFlight, "GBuffer Textures DS");
		m_SSBOLightDS = context.descriptorPool->AllocateSets(context, m_SSBOLightDSL, 1, "Light SSBO DS").front();
		m_vCameraMatricesDS = context.descriptorPool->AllocateSets(context, m_CameraMatricesDSL, context.maxFramesInFlight, "Camera Matrices DS");
		UpdateGBufferDescriptors(context, *createInfo.pGeometryPass, *createInfo.pDepthImages);

		DescriptorSetWriter writer{};
		for (uint32_t i{}; i < context.maxFramesInFlight; ++i)
		{
			writer
				.AddBufferInfo(m_vCameraMatrices[i], 0, sizeof(glm::mat4)*2)
				.WriteBuffers(m_vCameraMatricesDS[i], 0)
				.Execute(context);
		}
	}
}

void pompeii::LightingPass::Destroy()
{
	m_DeletionQueue.Flush();
}

void pompeii::LightingPass::UpdateGBufferDescriptors(const Context& context, const GeometryPass& pGeometryPass, const std::vector<Image>& depthImages) const
{
	DescriptorSetWriter writer{};
	for (uint32_t i{}; i < pGeometryPass.GetGBuffers().size(); ++i)
	{
		const GBuffer& gBuffer = pGeometryPass.GetGBuffer(i);

		writer
			.AddImageInfo(gBuffer.GetAlbedoOpacityImage().GetView(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_GBufferSampler)
			.WriteImages(m_vGBufferTexturesDS[i], 0)
			.Execute(context);

		writer
			.AddImageInfo(gBuffer.GetNormalImage().GetView(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_GBufferSampler)
			.WriteImages(m_vGBufferTexturesDS[i], 1)
			.Execute(context);

		writer
			.AddImageInfo(gBuffer.GetWorldPosImage().GetView(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_GBufferSampler)
			.WriteImages(m_vGBufferTexturesDS[i], 2)
			.Execute(context);

		writer
			.AddImageInfo(gBuffer.GetRoughnessMetallicImage().GetView(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_GBufferSampler)
			.WriteImages(m_vGBufferTexturesDS[i], 3)
			.Execute(context);

		writer
			.AddImageInfo(depthImages[i].GetView(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_GBufferSampler)
			.WriteImages(m_vGBufferTexturesDS[i], 4)
			.Execute(context);
	}
}
void pompeii::LightingPass::UpdateEnvironmentMap(const Context& context, const EnvironmentMap& envMap) const
{
	DescriptorSetWriter writer{};
	for (uint32_t i{}; i < m_vGBufferTexturesDS.size(); ++i)
	{
		writer
			.AddImageInfo(envMap.GetSkybox().GetView(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, envMap.GetSampler())
			.WriteImages(m_vGBufferTexturesDS[i], 5)
			.Execute(context);

		writer
			.AddImageInfo(envMap.GetDiffuseIrradianceMap().GetView(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, envMap.GetSampler())
			.WriteImages(m_vGBufferTexturesDS[i], 6)
			.Execute(context);

		writer
			.AddImageInfo(envMap.GetSpecularIrradianceMap().GetView(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, envMap.GetSampler())
			.WriteImages(m_vGBufferTexturesDS[i], 7)
			.Execute(context);

		writer
			.AddImageInfo(envMap.GetBRDFLut().GetView(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, envMap.GetSampler())
			.WriteImages(m_vGBufferTexturesDS[i], 8)
			.Execute(context);
	}
}

void pompeii::LightingPass::UpdateLightData(const Context& context, const std::vector<Light*>& data)
{
	uint32_t lightCount = static_cast<uint32_t>(data.size());

	// -- Prepare and Count Light Depth Maps --
	DescriptorSetWriter directionalWriter{};
	DescriptorSetWriter pointWriter{};
	uint32_t dirCount = 0;
	uint32_t pointCount = 0;

	// -- Extract GPU Light Data --
	std::vector<LightData> gpuData{};
	gpuData.reserve(data.size());
	for (Light* light : data)
	{
		if (light->vShadowMaps[context.currentFrame].GetHandle() == VK_NULL_HANDLE)
			continue;

		LightData ld{};
		ld.dirPos = light->dirPos;
		ld.type = light->type;
		ld.color = light->color;
		ld.intensity = light->luxLumen;

		int idx = 0;
		for (glm::mat4& v : light->viewMatrices)
		{
			ld.matrices[idx] = light->projMatrix * v;
			++idx;
		}

		if (light->type == LightType::Directional)
		{
			ld.depthIndex = dirCount;
			++dirCount;
			directionalWriter.AddImageInfo(light->vShadowMaps[context.currentFrame].GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_ShadowSampler);
		}
		else
		{
			ld.depthIndex = pointCount;
			++pointCount;
			pointWriter.AddImageInfo(light->vShadowMaps[context.currentFrame].GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_ShadowSampler);
		}

		gpuData.push_back(ld);
	}

	// -- Calculate buffer sizes --
	const VkDeviceSize totalLightSize = /*uint + 3padding*/4 * sizeof(uint32_t) + sizeof(LightData) * lightCount;
	if (m_SSBOLights.Size() != totalLightSize) // there new data changes 
	{
		// -- Destroy previous buffers --
		m_SSBOLights.Destroy(context);

		// -- Allocate Light Buffer --
		BufferAllocator bufferAlloc{};
		bufferAlloc
			.SetDebugName("SSBO (Light)")
			.SetUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
			.SetSize(static_cast<uint32_t>(totalLightSize))
			.AddInitialData(&lightCount, 0, sizeof(uint32_t))
			.AddInitialData(gpuData.data(), 4 * sizeof(uint32_t), sizeof(LightData) * lightCount)
			.HostAccess(true)
			.Allocate(context, m_SSBOLights);

		static uint32_t prevIdx = 0xFFFFFFFF;
		if (prevIdx != 0xFFFFFFFF)
		{
			m_DeletionQueue.Erase(prevIdx);
		}
		prevIdx = m_DeletionQueue.Push([&] { m_SSBOLights.Destroy(context); });
	}
	else // there is no new data changes
	{
		vmaCopyMemoryToAllocation(context.allocator, gpuData.data(), m_SSBOLights.GetMemoryHandle(), 4 * sizeof(uint32_t), sizeof(LightData) * lightCount);
	}

	// -- Update the Light Descriptor Sets --
	DescriptorSetWriter writer{};
	writer
		.AddBufferInfo(m_SSBOLights, 0, static_cast<uint32_t>(totalLightSize))
		.WriteBuffers(m_SSBOLightDS, 0)
		.Execute(context);
}
void pompeii::LightingPass::UpdateShadowMaps(const Context& context, const std::vector<LightItem>& lightItems)
{
	// -- Prepare and Count Light Depth Maps --
	DescriptorSetWriter directionalWriter{};
	DescriptorSetWriter pointWriter{};
	uint32_t dirCount = 0;
	uint32_t pointCount = 0;

	// -- Extract GPU Light Data --
	for (const LightItem& item : lightItems)
	{
		Light* light = item.light;
		if (light->vShadowMaps[context.currentFrame].GetHandle() == VK_NULL_HANDLE)
			continue;

		if (light->type == LightType::Directional)
		{
			++dirCount;
			directionalWriter.AddImageInfo(
				light->vShadowMaps[context.currentFrame].GetView(), 
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
				m_ShadowSampler);
		}
		else
		{
			++pointCount;
			pointWriter.AddImageInfo(
				light->vShadowMaps[context.currentFrame].GetView(), 
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
				m_ShadowSampler);
		}
	}

	auto& directionalImages = m_vUBODirLightMapDS[context.currentFrame];
	static std::vector<uint32_t> oldDirCount(context.maxFramesInFlight);
	if (dirCount > oldDirCount[context.currentFrame] || !directionalImages.GetHandle())
	{
		oldDirCount[context.currentFrame] = dirCount;

		VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
		variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
		variableCountInfo.descriptorSetCount = 1;
		uint32_t variableCount = dirCount;
		variableCountInfo.pDescriptorCounts = &variableCount;

		directionalImages = context.descriptorPool->AllocateSets(
			context, m_UBOLightMapDSL, 1, 
			"UBO Light Directional Shadow Maps",
			&variableCountInfo).front();
	}

	auto& pointImages = m_vUBOPointLightMapDS[context.currentFrame];
	static std::vector<uint32_t> oldPointCount(context.maxFramesInFlight);
	if (pointCount > oldPointCount[context.currentFrame] || !pointImages.GetHandle())
	{
		oldPointCount[context.currentFrame] = pointCount;

		VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
		variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
		variableCountInfo.descriptorSetCount = 1;
		uint32_t variableCount = pointCount;
		variableCountInfo.pDescriptorCounts = &variableCount;

		pointImages = context.descriptorPool->AllocateSets(
			context, m_UBOLightMapDSL, 1, 
			"UBO Light Directional Shadow Maps",
			&variableCountInfo).front();
	}

	// -- Update the Descriptor Sets with the Images --
	if (dirCount > 0)
		directionalWriter.WriteImages(directionalImages, 0, dirCount).Execute(context);
	if (pointCount > 0)
		pointWriter.WriteImages(pointImages, 0, pointCount).Execute(context);
}

void pompeii::LightingPass::Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, const Image& renderImage, const CameraData& camera) const
{
	// -- Update DS --
	struct cam
	{
		glm::mat4 view = camera.view;
		glm::mat4 proj = camera.proj;
	} camubo{};
	vmaCopyMemoryToAllocation(context.allocator, &camubo, m_vCameraMatrices[imageIndex].GetMemoryHandle(), 0, sizeof(camubo));

	// -- Setup Attachment --
	VkRenderingAttachmentInfo colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView = renderImage.GetView().GetHandle();
	colorAttachment.imageLayout = renderImage.GetCurrentLayout();
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.clearValue.color = { {0.f, 0.f, 0.f, 1.0f} };

	// -- Rendering Info --
	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea = VkRect2D{ VkOffset2D{0, 0}, renderImage.GetExtent2D() };
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;

	// -- Render --
	const VkCommandBuffer& vCmdBuffer = commandBuffer.GetHandle();
	Debugger::BeginDebugLabel(commandBuffer, "Lighting Pass", glm::vec4(0.6f, 0.2f, 0.8f, 1));
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
		VkRect2D scissor{};
		scissor.offset = { .x = 0, .y = 0 };
		scissor.extent = renderImage.GetExtent2D();
		Debugger::InsertDebugLabel(commandBuffer, "Bind Scissor", glm::vec4(1.f, 1.f, 0.2f, 1.f));
		vkCmdSetScissor(vCmdBuffer, 0, 1, &scissor);

		// -- Bind Descriptor Sets --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Cam Data", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout.GetHandle(), 0, 1, &m_vCameraMatricesDS[imageIndex].GetHandle(), 0, nullptr);
		Debugger::InsertDebugLabel(commandBuffer, "Bind Light Data", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout.GetHandle(), 1, 1, &m_SSBOLightDS.GetHandle(), 0, nullptr);
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout.GetHandle(), 2, 1, &m_vUBODirLightMapDS[imageIndex].GetHandle(), 0, nullptr);
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout.GetHandle(), 3, 1, &m_vUBOPointLightMapDS[imageIndex].GetHandle(), 0, nullptr);
		Debugger::InsertDebugLabel(commandBuffer, "Bind GBuffer", glm::vec4(0.f, 1.f, 1.f, 1.f));
		vkCmdBindDescriptorSets(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout.GetHandle(), 4, 1, &m_vGBufferTexturesDS[imageIndex].GetHandle(), 0, nullptr);

		// -- Draw Triangle --
		Debugger::InsertDebugLabel(commandBuffer, "Bind Pipeline (Lighting)", glm::vec4(0.2f, 0.4f, 1.f, 1.f));
		vkCmdBindPipeline(vCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetHandle());
		Debugger::InsertDebugLabel(commandBuffer, "Draw Full Screen Triangle", glm::vec4(0.4f, 0.8f, 1.f, 1.f));
		vkCmdDraw(commandBuffer.GetHandle(), 3, 1, 0, 0);
	}
	vkCmdEndRendering(vCmdBuffer);
	Debugger::EndDebugLabel(commandBuffer);
}
