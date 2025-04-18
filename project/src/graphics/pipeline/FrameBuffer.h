#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <vector>

// -- Forward Declarations --
namespace pom
{
	class RenderPass;
	class Image;
	struct Context;
}


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
		void Destroy(const Context& context) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkFramebuffer& GetHandle() const;

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
		FrameBufferBuilder& AddAttachment(const VkImageView& view);
		//! REQUIRED
		FrameBufferBuilder& SetExtent(uint32_t width, uint32_t height);

		void Build(const Context& context, FrameBuffer& frameBuffer) const;
		void Build(const Context& context, std::vector<FrameBuffer>& frameBuffers) const;

	private:
		VkFramebufferCreateInfo m_CreateInfo{};
		std::vector<VkImageView> m_vAttachments;
	};
}

#endif // FRAME_BUFFER_H