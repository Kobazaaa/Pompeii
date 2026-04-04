#ifndef POMPEII_BUFFER_INTERFACE_H
#define POMPEII_BUFFER_INTERFACE_H

// -- Pompeii Includes --
#include "PompeiiEnums.h"

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Buffer Description
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct BufferDesc
	{
		size_t size;
		BufferUsage usage;
		MemoryType memory;
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    IBuffer
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class IBuffer
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		virtual ~IBuffer() = default;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		[[nodiscard]] virtual size_t GetSize() const = 0;
		[[nodiscard]] virtual BufferUsage GetUsage() const = 0;
		[[nodiscard]] virtual MemoryType GetMemoryType() const = 0;
	};
}

#endif // POMPEII_BUFFER_INTERFACE_H
