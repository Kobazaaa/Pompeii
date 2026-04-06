#ifndef POMPEII_TEXTURE_H
#define POMPEII_TEXTURE_H

// -- Pompeii Includes --
#include "IImage.h"

namespace pompeii
{
    struct Texture
    {
        std::unique_ptr<IImage> image;
    };
}

#endif // POMPEII_TEXTURE_H
