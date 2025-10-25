#ifndef RENDER_INSTANCE_TYPE_H
#define RENDER_INSTANCE_TYPE_H

// -- Math --
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Light.h"

namespace pompeii
{
    struct RenderItem
    {
        Mesh* mesh;
        glm::mat4 transform;
    };
    struct LightItem
    {
        Light* light;
    };
}

#endif // RENDER_INSTANCE_TYPE_H