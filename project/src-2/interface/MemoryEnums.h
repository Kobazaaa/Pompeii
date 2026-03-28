#ifndef POMPEII_MEMORY_ENUMS_H
#define POMPEII_MEMORY_ENUMS_H

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Memory Type
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	enum class MemoryType
	{
		DeviceLocal,
		HostVisible,
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Shader Stage
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	enum class ShaderStage
	{
		Vertex,
		Fragment,
		Geometry,
		Compute
	};
	ENUM_AS_FLAGS(ShaderStage);
}

#endif // POMPEII_MEMORY_ENUMS_H
