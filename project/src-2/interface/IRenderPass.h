#ifndef POMPEII_RENDER_PASS_INTERFACE_H
#define POMPEII_RENDER_PASS_INTERFACE_H

// -- Forward Declarations --
namespace pompeii
{
    class IRenderer;
    class ICommandBuffer;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //? ~~    IRenderPass
    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    class IRenderPass
    {
    public:
        //--------------------------------------------------
        //    Constructor & Destructor
        //--------------------------------------------------
        virtual ~IRenderPass() = default;

        //--------------------------------------------------
        //    Rendering Functionality
        //--------------------------------------------------
        virtual void Setup(IRenderer* renderer) = 0;
        virtual void Resize(uint32_t width, uint32_t height) = 0;
        virtual void Execute(ICommandBuffer* cmd) = 0;
    };
}

#endif // POMPEII_RENDER_PASS_INTERFACE_H
