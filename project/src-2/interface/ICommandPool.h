#ifndef POMPEII_COMMAND_POOL_INTERFACE_H
#define POMPEII_COMMAND_POOL_INTERFACE_H

// -- Standard Library --
#include <memory>

// -- Pompeii Includes --
#include "ICommandBuffer.h"

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    ICommandPool
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class ICommandPool
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		virtual ~ICommandPool() = default;

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		virtual std::unique_ptr<ICommandBuffer> Allocate(CommandBufferLevel level) = 0;
		virtual void Free(ICommandBuffer* pCmdBuffer) = 0;
		virtual void Reset() = 0;
	};
}

#endif // POMPEII_COMMAND_POOL_INTERFACE_H
