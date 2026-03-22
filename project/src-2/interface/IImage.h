#ifndef POMPEII_IMAGE_INTERFACE_H
#define POMPEII_IMAGE_INTERFACE_H

// -- Pompeii Includes --
#include "Common.h"
#include "MemoryEnums.h"

namespace pompeii
{
    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //? ~~    Image Usage
    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    enum class ImageUsage
    {
        ColorAttachment         = BIT(0),
        DepthStencilAttachment  = BIT(1),
        Sampled                 = BIT(2),
        Storage                 = BIT(3),
    };
    ENUM_AS_FLAGS(ImageUsage);

    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //? ~~    Image Format
    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    enum class ImageFormat
    {
        RGBA8,
        RGBA16F,
        RGBA32F,
        Depth24Stencil8,
        Depth32F,
    };

    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //? ~~    Image Description
    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    struct ImageDesc
    {
        uint32_t width;
        uint32_t height;
        uint32_t depth = 1;
        uint32_t mipLevels = 1;
        ImageFormat format;
        ImageUsage usage;
        MemoryType memory;
    };

    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //? ~~    IImage
    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    class IImage
    {
    public:
        //--------------------------------------------------
        //    Constructor & Destructor
        //--------------------------------------------------
        virtual ~IImage() = default;

        //--------------------------------------------------
        //    Functionality
        //--------------------------------------------------
        virtual void SetData(const void* data, size_t size, uint32_t mipLevel = 0, uint32_t arrayLayer = 0) = 0;

        //--------------------------------------------------
        //    Accessors & Mutators
        //--------------------------------------------------
        [[nodiscard]] virtual uint32_t GetWidth() const = 0;
        [[nodiscard]] virtual uint32_t GetHeight() const = 0;
        [[nodiscard]] virtual ImageFormat GetFormat() const = 0;
        [[nodiscard]] virtual ImageUsage GetUsage() const = 0;
        [[nodiscard]] virtual MemoryType GetMemoryType() const = 0;

        //--------------------------------------------------
        //    Debug
        //--------------------------------------------------
        virtual void SetDebugName(const char* name) = 0;
    };
}

#endif // POMPEII_IMAGE_INTERFACE_H
