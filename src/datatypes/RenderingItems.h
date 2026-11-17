#ifndef RENDER_INSTANCE_TYPE_H
#define RENDER_INSTANCE_TYPE_H

// -- Math --
#include <glm/glm.hpp>
#include "Handle.h"

namespace pompeii
{
    struct RenderItem
    {
        MeshHandle mesh;
        glm::mat4 transform;
    };
    struct LightItem
    {
        LightHandle light;
    };
}

#endif // RENDER_INSTANCE_TYPE_H