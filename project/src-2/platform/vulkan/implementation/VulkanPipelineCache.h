#ifndef POMPEII_VULKAN_PIPELINE_CACHE_IMPLEMENTATION_H
#define POMPEII_VULKAN_PIPELINE_CACHE_IMPLEMENTATION_H

// -- Standard Library --
#include <unordered_map>
#include <memory>

// -- Pompeii Includes --
#include "IPipelineCache.h"
#include "VulkanPipeline.h"

namespace pompeii::vulkan
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    VulkanPipelineCache
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class VulkanPipelineCache final : public IPipelineCache
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit VulkanPipelineCache() = default;
		explicit VulkanPipelineCache(VulkanContext& context);
		~VulkanPipelineCache() override = default;

		VulkanPipelineCache(const VulkanPipelineCache& other) = default;
		VulkanPipelineCache(VulkanPipelineCache&& other) noexcept = default;
		VulkanPipelineCache& operator=(const VulkanPipelineCache& other) = default;
		VulkanPipelineCache& operator=(VulkanPipelineCache&& other) noexcept = default;

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		[[nodiscard]] IPipeline* GetOrCreate(const PipelineDesc& desc) override;

	private:
		VulkanContext* m_pContext{ nullptr };
		std::unordered_map<PipelineDesc, std::unique_ptr<VulkanPipeline>, PipelineDescHasher> m_PipelineMap{};
	};
}

#endif // POMPEII_VULKAN_PIPELINE_CACHE_IMPLEMENTATION_H
