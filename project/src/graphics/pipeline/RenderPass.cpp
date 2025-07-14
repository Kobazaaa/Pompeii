// -- Standard Library --
#include <stdexcept>

// -- Pompeii Includes --
#include "RenderPass.h"
#include "Context.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  RenderPass	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pompeii::RenderPass::Destroy(const Context& context)		const { vkDestroyRenderPass(context.device.GetHandle(), m_RenderPass, nullptr); }


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkRenderPass& pompeii::RenderPass::GetHandle()	const { return m_RenderPass; }
uint32_t pompeii::RenderPass::GetAttachmentCount()		const {	return m_AttachmentCount; }


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  RenderPassBuilder
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Builder
//--------------------------------------------------
pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::NewSubpass()
{
	m_SubPassIdx = static_cast<uint32_t>(m_vSubPasses.size());
	m_vSubPasses.emplace_back();
	m_vSubPasses.back().colorAttachmentCount = 0;
	m_vSubPasses.back().pColorAttachments = nullptr;
	m_vSubPasses.back().pDepthStencilAttachment = nullptr;
	m_vSubPasses.back().pResolveAttachments = nullptr;

	return *this;
}
pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::SetBindPoint(VkPipelineBindPoint bindPoint) { m_vSubPasses.back().pipelineBindPoint = bindPoint; return *this; }

pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::NewAttachment()								{ m_vAttachmentDescriptions.emplace_back(); return *this; }
pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::SetFormat(VkFormat format)					{ m_vAttachmentDescriptions.back().format = format; return *this; }
pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::SetSamples(VkSampleCountFlagBits samples)	{ m_vAttachmentDescriptions.back().samples = samples; return *this; }
pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::SetLoadStoreOp(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
{
	m_vAttachmentDescriptions.back().loadOp = loadOp;
	m_vAttachmentDescriptions.back().storeOp = storeOp;
	return *this;
}
pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::SetStencilLoadStoreOp(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
{
	m_vAttachmentDescriptions.back().stencilLoadOp = loadOp;
	m_vAttachmentDescriptions.back().stencilStoreOp = storeOp;
	return *this;
}
pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::SetInitialLayout(VkImageLayout layout)		{ m_vAttachmentDescriptions.back().initialLayout = layout; return *this; }
pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::SetFinalLayout(VkImageLayout layout)		{ m_vAttachmentDescriptions.back().finalLayout = layout; return *this; }

pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::AddSubpassColorAttachment(uint32_t attachment)
{
	if (m_vColorAttachmentRefs.size() <= m_SubPassIdx)
		m_vColorAttachmentRefs.emplace_back();
	m_vColorAttachmentRefs.back().emplace_back(attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	m_vSubPasses.back().colorAttachmentCount = static_cast<uint32_t>(m_vColorAttachmentRefs.back().size());
	m_vSubPasses.back().pColorAttachments = m_vColorAttachmentRefs.back().data();
	return *this;
}
pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::SetSubpassResolveAttachment(uint32_t attachment)
{
	m_ResolveAttachmentRef.emplace_back(attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	m_vSubPasses.back().pResolveAttachments = &m_ResolveAttachmentRef.back();
	return *this;
}
pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::SetSubpassDepthAttachment(uint32_t attachment)
{
	m_vDepthAttachmentRefs.emplace_back(attachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	m_vSubPasses.back().pDepthStencilAttachment = &m_vDepthAttachmentRefs.back();
	return *this;
}

pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::NewDependency()								{ m_vSubPassDependencies.emplace_back(); return *this; }
pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::AddDependencyFlag(VkDependencyFlags flags)	{ m_vSubPassDependencies.back().dependencyFlags = flags; return *this; }
pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::SetSrcSubPass(uint32_t subpass)				{ m_vSubPassDependencies.back().srcSubpass = subpass; return *this; }
pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::SetDstSubPass(uint32_t subpass)				{ m_vSubPassDependencies.back().dstSubpass = subpass; return *this; }
pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::SetSrcMasks(VkPipelineStageFlags stageFlags, VkAccessFlags accessFlags)
{
	m_vSubPassDependencies.back().srcStageMask = stageFlags;
	m_vSubPassDependencies.back().srcAccessMask = accessFlags;
	return *this;
}
pompeii::RenderPassBuilder& pompeii::RenderPassBuilder::SetDstMasks(VkPipelineStageFlags stageFlags, VkAccessFlags accessFlags)
{
	m_vSubPassDependencies.back().dstStageMask = stageFlags;
	m_vSubPassDependencies.back().dstAccessMask = accessFlags;
	return *this;
}

void pompeii::RenderPassBuilder::Build(const Context& context, RenderPass& renderPass) const
{
	renderPass.m_AttachmentCount = static_cast<uint32_t>(m_vAttachmentDescriptions.size());
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = renderPass.m_AttachmentCount;
	renderPassInfo.pAttachments = m_vAttachmentDescriptions.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(m_vSubPasses.size());
	renderPassInfo.pSubpasses = m_vSubPasses.data();
	renderPassInfo.dependencyCount = static_cast<uint32_t>(m_vSubPassDependencies.size());
	renderPassInfo.pDependencies = m_vSubPassDependencies.data();

	if (vkCreateRenderPass(context.device.GetHandle(), &renderPassInfo, nullptr, &renderPass.m_RenderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to created Render Pass!");
}
