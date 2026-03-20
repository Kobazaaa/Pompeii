#ifndef POMPEII_IBUFFER_INTERFACE_H
#define POMPEII_IBUFFER_INTERFACE_H

// -- Pompeii Includes --
#include "Common.h"

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Buffer Usage
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	enum class BufferUsage
	{
		Vertex	= BIT(0),
		Index	= BIT(1),
		Uniform = BIT(2),
		Storage = BIT(3),
		Staging = BIT(4)
	};
	ENUM_AS_FLAGS(BufferUsage);

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Memory
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	enum class MemoryType
	{
		DeviceLocal,
		HostVisible,
	};

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
		//    Functionality
		//--------------------------------------------------
		virtual void SetData(const void* data, size_t size, size_t offset = 0) = 0;
		virtual void* Map() = 0;
		virtual void  Unmap() = 0;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		virtual size_t GetSize() const = 0;
		virtual BufferUsage GetUsage() const = 0;
		virtual MemoryType GetMemoryType() const = 0;

		//--------------------------------------------------
		//    Debug
		//--------------------------------------------------
		virtual void SetDebugName(const char* name) = 0;
	};
}

#endif // POMPEII_IBUFFER_INTERFACE_H
