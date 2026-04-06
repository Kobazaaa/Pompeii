#ifndef POMPEII_VULKAN_PIPELINE_IMPLEMENTATION_H
#define POMPEII_VULKAN_PIPELINE_IMPLEMENTATION_H

// -- Pompeii Includes --
#include "IPipeline.h"

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Forward Declaration --
namespace pompeii::vulkan
{
    struct VulkanContext;
}

namespace pompeii::vulkan
{
	class VulkanPipeline final : public IPipeline
    {
    public:
        //--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
        VulkanPipeline(VulkanContext& context, const PipelineDesc& desc);
        ~VulkanPipeline() override;

        //--------------------------------------------------
		//    Operator Overloads
		//--------------------------------------------------
		operator VkPipeline() const;
		operator VkPipelineLayout() const;

    private:
        void CreatePipelineLayout(const PipelineDesc& desc);
        void CreatePipeline(const PipelineDesc& desc);

        VulkanContext* m_pContext   { nullptr };

        VkPipeline m_Pipeline       { VK_NULL_HANDLE };
        VkPipelineLayout m_Layout   { VK_NULL_HANDLE };
    };
}

#endif // POMPEII_VULKAN_PIPELINE_IMPLEMENTATION_H
