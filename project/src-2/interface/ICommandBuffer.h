#ifndef POMPEII_COMMAND_BUFFER_INTERFACE_H
#define POMPEII_COMMAND_BUFFER_INTERFACE_H

// -- Standard Library --
#include <cstdint>

// -- Math Includes --
#include "glm/vec4.hpp"

// -- Pompeii Includes --
#include "RenderingInfo.h"

// -- Forward Declarations --
namespace pompeii
{
	class FrameContext;
	class ICommandPool;

	class IBuffer;
    class IImage;
    class IPipeline;

	struct Material;
	class Mesh;
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
		//    Accessors
		//--------------------------------------------------
        virtual ICommandPool* GetOwnerPool() const = 0;

        //--------------------------------------------------
        //    Functionality
        //--------------------------------------------------
        virtual void Begin(bool oneTime = false) = 0;
        virtual void End() = 0;
        virtual void Submit(FrameContext& frameCtx) = 0;
        virtual void Submit() = 0;
        virtual void Clear() = 0;

        //--------------------------------------------------
        //    Commands
        //--------------------------------------------------
        // Data Commands
        virtual void UploadDataToBuffer(IBuffer& buffer, const void* data, size_t size, size_t offset) = 0;
        virtual void UploadDataToImage(IImage& image, const void* data, size_t size) = 0;

        // Copy Commands
        virtual void CopyBuffer(IBuffer& src, IBuffer& dst, size_t size, uint32_t srcOffset = 0, uint32_t dstOffset = 0) = 0;
        virtual void CopyImage(IImage& src, IImage& dst) = 0;
        virtual void CopyBufferToImage(IBuffer& src, IImage& dst) = 0;
        virtual void Blit(IImage& src, IImage& dst) = 0;

        // Image Commands
        virtual void GenerateMips(IImage& src) = 0;

        // Rendering
        virtual void SetViewport(float x, float y, float w, float h) = 0;
        virtual void SetScissors(int32_t x, int32_t y, uint32_t w, uint32_t h) = 0;
        virtual void BeginRendering(const RenderingInfo& info) = 0;
        virtual void EndRendering() = 0;

        // Drawing
        virtual void BindPipeline(IPipeline& pipeline) = 0;
        virtual void DrawMesh(Mesh& mesh) = 0;
        virtual void Draw(uint32_t vertexCount) = 0;
    };
}

#endif // POMPEII_COMMAND_BUFFER_INTERFACE_H
