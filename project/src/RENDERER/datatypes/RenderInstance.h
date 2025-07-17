#ifndef RENDER_INSTANCE_TYPE_H
#define RENDER_INSTANCE_TYPE_H

// -- Math --
#include <glm/glm.hpp>
#include "Model.h"

namespace pompeii
{
    struct RenderInstance
    {
        ModelHandle handle;
        glm::mat4 transform;
    };
    struct RenderDrawContext
	{
        const std::vector<RenderInstance>& instances;
        std::function<const ModelGPU& (ModelHandle)> resolveModel;
    };
}

#endif // RENDER_INSTANCE_TYPE_H