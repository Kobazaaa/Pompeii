#ifndef POMPEII_POMPEII_ENUMS_H
#define POMPEII_POMPEII_ENUMS_H

// -- Pompeii Includes --
#include "BitOperations.h"
#include "EnumUtilities.h"

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
	//? ~~    Shaders
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	enum class ShaderStage
	{
		Vertex,
		Fragment,
		Geometry,
		Compute
	};
	ENUM_AS_FLAGS(ShaderStage);

    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Image
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    enum class ImageUsage
    {
        ColorAttachment = BIT(0),
        DepthStencilAttachment = BIT(1),
        Sampled = BIT(2),
        Storage = BIT(3),
    };
    ENUM_AS_FLAGS(ImageUsage);

    enum class ImageFormat
    {
        RGBA8,
        RGBA16F,
        RGBA32F,
        Depth24Stencil8,
        Depth32F,
    };

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Buffer
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	enum class BufferUsage
	{
		Vertex = BIT(0),
		Index = BIT(1),
		Uniform = BIT(2),
		Storage = BIT(3),
		Staging = BIT(4)
	};
	ENUM_AS_FLAGS(BufferUsage);


    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Command Buffers
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    enum class CommandBufferLevel
    {
        Primary,
        Secondary
    };


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Pipeline
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	enum class CullMode
	{
		None,
		Front,
		Back,
		Both
	};

	enum class FrontFace
	{
		Clockwise,
		CounterClockwise
	};

	enum class PolygonMode
	{
		Fill,
	};

	enum class PrimitiveTopology
	{
		TriangleList,
	};

	enum class DepthCompareOp
	{
		Never,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Vertex
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	enum class VertexInputRate
	{
		Vertex,
		Instance
	};

	enum class VertexFormat
	{
		Float,
		Float2,
		Float3,
		Float4
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Load/Store Ops
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	enum class LoadOp
	{
		Load,
		Clear,
		DontCare
	};

	enum class StoreOp
	{
		Store,
		DontCare
	};
}

#endif // POMPEII_POMPEII_ENUMS_H
