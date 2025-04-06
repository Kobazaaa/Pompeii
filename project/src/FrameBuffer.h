#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include "Device.h"
#include "Image.h"
#include "RenderPass.h"

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  FrameBuffer	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class FrameBuffer final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		FrameBuffer() = default;
		void Destroy(const Device& device) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkFramebuffer& GetBuffer() const;

	private:
		VkFramebuffer m_Buffer;
		friend class FrameBufferBuilder;
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  FrameBufferBuilder	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class FrameBufferBuilder final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		FrameBufferBuilder();


		//--------------------------------------------------
		//    Builder
		//--------------------------------------------------

		//! REQUIRED
		FrameBufferBuilder& SetRenderPass(const RenderPass& renderPass);
		FrameBufferBuilder& AddAttachment(const Image& image);
		//! REQUIRED
		FrameBufferBuilder& SetExtent(uint32_t width, uint32_t height);

		void Build(const Device& device, FrameBuffer& frameBuffer) const;
		void Build(const Device& device, std::vector<FrameBuffer>& frameBuffers) const;

	private:
		VkFramebufferCreateInfo m_CreateInfo{};
		std::vector<VkImageView> m_vAttachments;
	};
}

#endif // FRAME_BUFFER_H