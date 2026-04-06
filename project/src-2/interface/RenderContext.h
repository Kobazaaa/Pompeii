#ifndef POMPEII_RENDER_CONTEXT_H
#define POMPEII_RENDER_CONTEXT_H

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    RenderContext
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct RenderContext
	{
		IImage& backBufferImage;
		IResourceManager& resourceManager;
		IPipelineCache& pipelineCache;
	};
}

#endif // POMPEII_RENDER_CONTEXT_H
