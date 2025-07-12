// -- Standard Library --
#include <ranges>

// -- Pompeii Includes --
#include "GBuffer.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  GBuffer	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::GBuffer::GBuffer(GBuffer&& other) noexcept
{
	m_Albedo_Opacity = std::move(other.m_Albedo_Opacity);
	m_Normal = std::move(other.m_Normal);
	m_WorldPos = std::move(other.m_WorldPos);
	m_Roughness_Metallic = std::move(other.m_Roughness_Metallic);
	m_vAllImages = std::move(other.m_vAllImages);
	other.m_vAllImages.clear();
	m_vRenderingAttachments = std::move(other.m_vRenderingAttachments);
	other.m_vRenderingAttachments.clear();
	m_Extent = std::move(other.m_Extent);
}
pom::GBuffer& pom::GBuffer::operator=(GBuffer&& other) noexcept
{
	if (this == &other)
		return *this;
	m_Albedo_Opacity = std::move(other.m_Albedo_Opacity);
	m_Normal = std::move(other.m_Normal);
	m_WorldPos = std::move(other.m_WorldPos);
	m_Roughness_Metallic = std::move(other.m_Roughness_Metallic);
	m_vAllImages = std::move(other.m_vAllImages);
	other.m_vAllImages.clear();
	m_vRenderingAttachments = std::move(other.m_vRenderingAttachments);
	other.m_vRenderingAttachments.clear();
	m_Extent = std::move(other.m_Extent);
	return *this;
}

void pom::GBuffer::Initialize(const Context& context, VkExtent2D size)
{
	m_Extent = size;

	CreateImage(context, m_Albedo_Opacity,
		size, VK_FORMAT_R32G32B32A32_SFLOAT,
		"GBuffer - Albedo_Opacity");
	CreateImage(context, m_Normal,
		size, VK_FORMAT_R16G16B16A16_UNORM,
		"GBuffer - Normal");
	CreateImage(context, m_WorldPos,
		size, VK_FORMAT_R32G32B32A32_SFLOAT,
		"GBuffer - WorldPos");
	CreateImage(context, m_Roughness_Metallic,
		size, VK_FORMAT_R8G8_UNORM,
		"GBuffer - Roughness_Metallic");
}
void pom::GBuffer::Destroy(const Context& context)
{
	for (Image* const& image : std::views::reverse(m_vAllImages))
	{
		image->Destroy(context);
	}
	m_vAllImages.clear();
	m_vRenderingAttachments.clear();
}

void pom::GBuffer::Resize(const Context& context, VkExtent2D size)
{
	Destroy(context);
	Initialize(context, size);
	m_Extent = size;
}

void pom::GBuffer::TransitionBufferWriting(const CommandBuffer& commandBuffer)
{
	m_vRenderingAttachments.clear();
	m_vRenderingAttachments.reserve(m_vAllImages.size());
	for (Image*& image : m_vAllImages)
	{
		image->TransitionLayout(commandBuffer,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, 1, 0, 1);
		AddRenderingAttachment(*image);
	}
}
void pom::GBuffer::TransitionBufferSampling(const CommandBuffer& commandBuffer)
{
	for (Image*& image : m_vAllImages)
	{
		image->TransitionLayout(commandBuffer,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			0, 1, 0, 1);
	}
}

const std::vector<VkRenderingAttachmentInfo>& pom::GBuffer::GetRenderingAttachments() const { return m_vRenderingAttachments; }
uint32_t pom::GBuffer::GetAttachmentCount()											  const { return static_cast<uint32_t>(m_vRenderingAttachments.size()); }

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
std::vector<VkFormat> pom::GBuffer::GetAllFormats() const
{
	return {m_Albedo_Opacity.GetFormat(), m_Normal.GetFormat(), m_WorldPos.GetFormat(), m_Roughness_Metallic.GetFormat()};
}
VkExtent2D pom::GBuffer::GetExtent() const { return m_Extent; }

const pom::Image& pom::GBuffer::GetAlbedoOpacityImage()			const {	return m_Albedo_Opacity; }
const pom::Image& pom::GBuffer::GetNormalImage()				const { return m_Normal; }
const pom::Image& pom::GBuffer::GetWorldPosImage()				const { return m_WorldPos; }
const pom::Image& pom::GBuffer::GetRoughnessMetallicImage()		const { return m_Roughness_Metallic; }

// -- Helper --
void pom::GBuffer::CreateImage(const Context& context, Image& image, VkExtent2D size, VkFormat format, const char* pName)
{
	ImageBuilder builder{};
	builder
		.SetDebugName(pName)
		.SetWidth(size.width)
		.SetHeight(size.height)
		.SetFormat(format)
		.SetTiling(VK_IMAGE_TILING_OPTIMAL)
		.SetUsageFlags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
		.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		.Build(context, image);
	image.CreateView(context, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);
	m_vAllImages.emplace_back(&image);
}
void pom::GBuffer::AddRenderingAttachment(const Image& image)
{
	VkRenderingAttachmentInfo attachmentInfo{};
	attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	attachmentInfo.imageView = image.GetView().GetHandle();
	attachmentInfo.imageLayout = image.GetCurrentLayout();
	attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentInfo.clearValue.color = { {0.f, 0.f, 0.f, 1.0f} };

	m_vRenderingAttachments.emplace_back(attachmentInfo);
}
