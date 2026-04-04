#ifndef POMPEII_IMAGE_INTERFACE_H
#define POMPEII_IMAGE_INTERFACE_H

// -- Pompeii Includes --
#include "PompeiiEnums.h"

namespace pompeii
{
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
        //    Accessors & Mutators
        //--------------------------------------------------
        [[nodiscard]] virtual uint32_t GetWidth() const = 0;
        [[nodiscard]] virtual uint32_t GetHeight() const = 0;
        [[nodiscard]] virtual ImageFormat GetFormat() const = 0;
        [[nodiscard]] virtual ImageUsage GetUsage() const = 0;
        [[nodiscard]] virtual MemoryType GetMemoryType() const = 0;
    };
}

#endif // POMPEII_IMAGE_INTERFACE_H
