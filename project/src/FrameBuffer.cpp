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
void pom::FrameBuffer::Destroy(const Context& context) const { vkDestroyFramebuffer(context.device.GetHandle(), m_Buffer, nullptr); }

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkFramebuffer& pom::FrameBuffer::GetHandle() const { return m_Buffer; }



//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  FrameBuffer	Builder
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::FrameBufferBuilder::FrameBufferBuilder()
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
pom::FrameBufferBuilder& pom::FrameBufferBuilder::SetRenderPass(const RenderPass& renderPass)
{
	m_CreateInfo.renderPass = renderPass.GetHandle();
	return *this;
}
pom::FrameBufferBuilder& pom::FrameBufferBuilder::AddAttachment(const Image& image)
{
	m_vAttachments.push_back(image.GetViewHandle());
	m_CreateInfo.attachmentCount = static_cast<uint32_t>(m_vAttachments.size());
	m_CreateInfo.pAttachments = m_vAttachments.data();
	return *this;
}
pom::FrameBufferBuilder& pom::FrameBufferBuilder::SetExtent(uint32_t width, uint32_t height)
{
	m_CreateInfo.width = width;
	m_CreateInfo.height = height;
	return *this;
}

void pom::FrameBufferBuilder::Build(const Context& context, FrameBuffer& frameBuffer) const
{
	if (vkCreateFramebuffer(context.device.GetHandle(), &m_CreateInfo, nullptr, &frameBuffer.m_Buffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Framebuffer!");
}

void pom::FrameBufferBuilder::Build(const Context& context, std::vector<FrameBuffer>& frameBuffers) const
{
	frameBuffers.emplace_back();
	Build(context, frameBuffers.back());
}
