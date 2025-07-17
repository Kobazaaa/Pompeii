// -- Standard Library --
#include <stdexcept>

// -- Pompeii Includes --
#include "FrameBuffer.h"
#include "Context.h"
#include "RenderPass.h"
#include "Image.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  FrameBuffer	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
{
	m_Buffer = other.m_Buffer;
	other.m_Buffer = VK_NULL_HANDLE;
	m_Extent = std::move(other.m_Extent);
}
pompeii::FrameBuffer& pompeii::FrameBuffer::operator=(FrameBuffer&& other) noexcept
{
	if (this == &other)
		return *this;
	m_Buffer = std::move(other.m_Buffer);
	other.m_Buffer = VK_NULL_HANDLE;
	m_Extent = std::move(other.m_Extent);
	return *this;
}

void pompeii::FrameBuffer::Destroy(const Context& context) const { vkDestroyFramebuffer(context.device.GetHandle(), m_Buffer, nullptr); }

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkFramebuffer& pompeii::FrameBuffer::GetHandle()	const { return m_Buffer; }
VkExtent2D pompeii::FrameBuffer::GetExtent()			const {	return m_Extent; }


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  FrameBuffer	Builder
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::FrameBufferBuilder::FrameBufferBuilder()
{
	m_CreateInfo = {};
    m_CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;                 // CAN'T CHANGE
    m_CreateInfo.renderPass = VK_NULL_HANDLE;                                       //! REQUIRED CHANGE
    m_CreateInfo.attachmentCount = 0;                                               //? CAN CHANGE
    m_CreateInfo.pAttachments = nullptr;                                            //? CAN CHANGE
    m_CreateInfo.width = 0;                                                         //! REQUIRED CHANGE
    m_CreateInfo.height = 0;                                                        //! REQUIRED CHANGE
    m_CreateInfo.layers = 1;														// CAN'T CHANGE
}


//--------------------------------------------------
//    Builder
//--------------------------------------------------
pompeii::FrameBufferBuilder& pompeii::FrameBufferBuilder::SetRenderPass(const RenderPass& renderPass)
{
	m_CreateInfo.renderPass = renderPass.GetHandle();
	return *this;
}
pompeii::FrameBufferBuilder& pompeii::FrameBufferBuilder::AddAttachment(const VkImageView& view)
{
	m_vAttachments.push_back(view);
	m_CreateInfo.attachmentCount = static_cast<uint32_t>(m_vAttachments.size());
	m_CreateInfo.pAttachments = m_vAttachments.data();
	return *this;
}
pompeii::FrameBufferBuilder& pompeii::FrameBufferBuilder::SetExtent(uint32_t width, uint32_t height)
{
	m_CreateInfo.width = width;
	m_CreateInfo.height = height;
	return *this;
}

void pompeii::FrameBufferBuilder::Build(const Context& context, FrameBuffer& frameBuffer) const
{
	if (vkCreateFramebuffer(context.device.GetHandle(), &m_CreateInfo, nullptr, &frameBuffer.m_Buffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Framebuffer!");

	frameBuffer.m_Extent = { .width = m_CreateInfo.width, .height = m_CreateInfo.height };
}

void pompeii::FrameBufferBuilder::Build(const Context& context, std::vector<FrameBuffer>& frameBuffers) const
{
	frameBuffers.emplace_back();
	Build(context, frameBuffers.back());
}
