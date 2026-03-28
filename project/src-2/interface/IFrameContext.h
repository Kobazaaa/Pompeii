#ifndef POMPEII_INTERFACE_FRAME_CONTEXT_H
#define POMPEII_INTERFACE_FRAME_CONTEXT_H

// -- Standard Library --
#include <memory>

// -- Pompeii Includes --
#include "ICommandBuffer.h"

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    IFrameContext
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct IFrameContext
	{
		IFrameContext() = default;
		virtual ~IFrameContext() = default;
		IFrameContext(IFrameContext&&) noexcept = default;
		IFrameContext& operator=(IFrameContext&&) noexcept = default;
		std::unique_ptr<ICommandBuffer> CommandBuffer { nullptr };
	};
}

#endif // POMPEII_INTERFACE_FRAME_CONTEXT_H
