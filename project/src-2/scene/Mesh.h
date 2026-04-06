#ifndef POMPEII_MESH_H
#define POMPEII_MESH_H

// -- Standard Library --
#include <memory>

// -- Pompeii Includes --
#include "IBuffer.h"

namespace pompeii
{
    struct MeshDesc
    {
        const void* vertexData;
        size_t vertexSize;

        const void* indexData;
        size_t indexSize;
    };

    struct Mesh
    {
        std::unique_ptr<IBuffer> vertexBuffer;
        std::unique_ptr<IBuffer> indexBuffer;
        uint32_t indexCount;
    };
}

#endif // POMPEII_MESH_H
