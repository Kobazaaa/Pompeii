#ifndef POMPEII_INTERFACE_FRAME_CONTEXT_H
#define POMPEII_INTERFACE_FRAME_CONTEXT_H

// -- Standard Library --
#include <memory>

// -- Pompeii Includes --
#include "ICommandBuffer.h"

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    FrameContext
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class FrameContext
	{
	public:
		FrameContext(std::unique_ptr<ICommandBuffer> commandBuffer, uint32_t index)
			: m_pCommandBuffer(std::move(commandBuffer))
			, m_FrameIndex(index)
		{ }
		FrameContext() = default;
		virtual ~FrameContext() = default;
		FrameContext(FrameContext&&) noexcept = default;
		FrameContext& operator=(FrameContext&&) noexcept = default;

		ICommandBuffer* GetCommandBuffer() const { return m_pCommandBuffer.get(); };
		uint32_t FrameIndex() const { return m_FrameIndex; };
		bool IsValid() const { return m_pCommandBuffer != nullptr; }

	private:
		std::unique_ptr<ICommandBuffer> m_pCommandBuffer;
		uint32_t m_FrameIndex;
	};
}

#endif // POMPEII_INTERFACE_FRAME_CONTEXT_H
