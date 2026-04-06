#ifndef POMPEII_PIPELINE_CACHE_INTERFACE_H
#define POMPEII_PIPELINE_CACHE_INTERFACE_H

// -- Pompeii Includes --
#include "IPipeline.h"

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    IPipelineCache
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class IPipelineCache
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		virtual ~IPipelineCache() = default;

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		[[nodiscard]] virtual IPipeline* GetOrCreate(const PipelineDesc& desc) = 0;
	};
}

#endif // POMPEII_PIPELINE_CACHE_INTERFACE_H
