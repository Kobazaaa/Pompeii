#ifndef POMPEII_RENDERER_INTERFACE_H
#define POMPEII_RENDERER_INTERFACE_H

// -- Standard Library --
#include <memory>

// -- Forward Declarations --
namespace pompeii
{
    class IRenderPass;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //? ~~    IRenderer
    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    class IRenderer
    {
    public:
        //--------------------------------------------------
        //    Constructor & Destructor
        //--------------------------------------------------
        virtual ~IRenderer() = default;

        //--------------------------------------------------
        //    Functionality
        //--------------------------------------------------
        virtual void AddRenderPass(std::unique_ptr<IRenderPass> pass) = 0;

        virtual void BeginFrame() = 0;
        virtual void Execute() = 0;
        virtual void EndFrame() = 0;
    };
}

#endif // POMPEII_RENDERER_INTERFACE_H
