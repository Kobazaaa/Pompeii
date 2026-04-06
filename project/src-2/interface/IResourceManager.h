#ifndef POMPEII_RESOURCE_MANAGER_INTERFACE_H
#define POMPEII_RESOURCE_MANAGER_INTERFACE_H

// -- Pompeii Includes --
#include "IBuffer.h"
#include "IImage.h"
#include "IShader.h"

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    IResourceManager
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class IResourceManager
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		virtual ~IResourceManager() = default;

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		[[nodiscard]] virtual std::unique_ptr<IImage> CreateImage(const ImageDesc& desc) = 0;
		virtual void UploadImage(IImage& image, const void* data, size_t size) = 0;

		[[nodiscard]] virtual std::unique_ptr<IBuffer> CreateBuffer(const BufferDesc& desc) = 0;
		virtual void UploadBuffer(IBuffer& buffer, const void* data, size_t size, size_t offset) = 0;

		[[nodiscard]] virtual std::unique_ptr<IShader> CreateShader(const ShaderDesc& desc) = 0;

	};
}

#endif // POMPEII_RESOURCE_MANAGER_INTERFACE_H
