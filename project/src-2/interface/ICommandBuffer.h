#ifndef POMPEII_COMMAND_BUFFER_INTERFACE_H
#define POMPEII_COMMAND_BUFFER_INTERFACE_H

// -- Forward Declarations --
namespace pompeii
{
    class IBuffer;
    class IImage;
}

namespace pompeii
{
    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //? ~~    ICommandBuffer
    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    class ICommandBuffer
    {
    public:
        //--------------------------------------------------
        //    Constructor & Destructor
        //--------------------------------------------------
        virtual ~ICommandBuffer() = default;

        //--------------------------------------------------
        //    Functionality
        //--------------------------------------------------
        virtual void Begin() = 0;
        virtual void End() = 0;

        //--------------------------------------------------
        //    Commands
        //--------------------------------------------------
        virtual void Copy(IBuffer* src, IBuffer* dst, size_t size, size_t srcOffset = 0, size_t dstOffset = 0) = 0;
        virtual void Blit(IImage* src, IImage* dst) = 0;
    };
}

#endif // POMPEII_COMMAND_BUFFER_INTERFACE_H
