// -- Pompeii::vulkan Includes --
#include "VulkanPipelineCache.h"


//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::vulkan::VulkanPipelineCache::VulkanPipelineCache(VulkanContext& context)
	: m_pContext(&context)
{ }

//--------------------------------------------------
//    Functionality
//--------------------------------------------------
pompeii::IPipeline* pompeii::vulkan::VulkanPipelineCache::GetOrCreate(const PipelineDesc& desc)
{
	if (m_PipelineMap.contains(desc))
		return m_PipelineMap[desc].get();

	auto pair = m_PipelineMap.insert({desc, std::make_unique<VulkanPipeline>(*m_pContext, desc) });
	return pair.first->second.get();
}

