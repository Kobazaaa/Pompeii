#ifndef POMPEII_RENDERING_INFO_H
#define POMPEII_RENDERING_INFO_H

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "PompeiiEnums.h"

// -- Forward Declarations --
namespace pompeii
{
	class IImage;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    RenderingAttachment
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct RenderingAttachment
	{
		IImage& image;

		LoadOp loadOp = LoadOp::Clear;
		StoreOp storeOp = StoreOp::Store;

		glm::vec4 clearValue = glm::vec4(0.0f);
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    RenderingInfo
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct RenderingInfo
	{
		std::vector<RenderingAttachment> colorAttachments;
		RenderingAttachment* depthAttachment = nullptr;

		uint32_t width = 0;
		uint32_t height = 0;
	};
}

#endif // POMPEII_RENDERING_INFO_H
