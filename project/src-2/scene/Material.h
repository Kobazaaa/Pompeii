#ifndef POMPEII_MATERIAL_H
#define POMPEII_MATERIAL_H

// -- Standard Library --
#include <filesystem>
#include <unordered_map>

// -- Pompeii Includes --
#include "GPUHandle.h"

namespace pompeii
{
    class IPipeline;

    struct BindingValue {
        enum Type { Texture, Buffer, Sampler } type;

        union {
            TextureHandle texture;
            BufferHandle buffer;
        };
    };
    using BindingMap = std::unordered_map<uint32_t, BindingValue>;


    struct MaterialDesc
    {
        IPipeline* pipeline = nullptr;
    };

    struct Material
    {
        IPipeline* pipeline = nullptr;
        BindingMap bindings;
    };
}

#endif // POMPEII_MATERIAL_H
