#ifndef POMPEII_RENDERER_INTERFACE_H
#define POMPEII_RENDERER_INTERFACE_H

// -- Standard Library --
#include <memory>

// -- Forward Declarations --
namespace pompeii
{
    class IRenderPass;
    class IBuffer;
    class IImage;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //? ~~    Render Desc
    //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    struct RenderDesc
    {
        uint32_t width;
        uint32_t height;
        uint32_t sampleCount;
	};

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
        virtual void Shutdown() = 0;

        //--------------------------------------------------
        //    Functionality
        //--------------------------------------------------
        virtual void AddRenderPass(std::unique_ptr<IRenderPass> pass) = 0;

        virtual void BeginFrame() = 0;
        virtual void Execute() = 0;
        virtual void EndFrame() = 0;

        //--------------------------------------------------
		//    Data Handling
		//--------------------------------------------------
        virtual void UploadImage(IImage& image, void* data, size_t size) = 0;
        virtual void UploadBuffer(IBuffer& image, void* data, size_t size, size_t offset) = 0;
    };
}

#endif // POMPEII_RENDERER_INTERFACE_H
