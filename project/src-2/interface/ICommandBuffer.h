#ifndef POMPEII_COMMAND_BUFFER_INTERFACE_H
#define POMPEII_COMMAND_BUFFER_INTERFACE_H

// -- Standard Library --
#include <cstdint>

// -- Forward Declarations --
namespace pompeii
{
	class ICommandPool;
	class Mesh;
    class IBuffer;
    class IImage;
}

namespace pompeii
{
    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //? ~~    CommandBufferLevel
    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    enum class CommandBufferLevel
    {
        Primary,
        Secondary
	};

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
		//    Accessors
		//--------------------------------------------------
        virtual ICommandPool* GetOwnerPool() const = 0;

        //--------------------------------------------------
        //    Functionality
        //--------------------------------------------------
        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual void Clear() = 0;

        //--------------------------------------------------
        //    Commands
        //--------------------------------------------------
        // Copy Commands
        virtual void CopyBuffer(IBuffer* src, IBuffer* dst, size_t size, uint32_t srcOffset = 0, uint32_t dstOffset = 0) = 0;
        virtual void CopyImage(IImage* src, IImage* dst) = 0;
        virtual void CopyBufferToImage(IBuffer* src, IImage* dst) = 0;
        virtual void Blit(IImage* src, IImage* dst) = 0;

        // Image Commands
        virtual void GenerateMips(IImage* src) = 0;

        // Drawing
        virtual void DrawMesh(Mesh* pMesh) = 0;
    };
}

#endif // POMPEII_COMMAND_BUFFER_INTERFACE_H
