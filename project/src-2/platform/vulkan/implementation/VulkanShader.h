#ifndef POMPEII_VULKAN_SHADER_H
#define POMPEII_VULKAN_SHADER_H

// -- Pompeii Includes --
#include "IShader.h"

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Forward Declaration --
namespace pompeii::vulkan
{
    struct VulkanContext;
}

namespace pompeii::vulkan
{
    class VulkanShader final : public IShader
    {
    public:
	    //--------------------------------------------------
        //    Constructor & Destructor
        //--------------------------------------------------
        explicit VulkanShader(VulkanContext& context, const ShaderDesc& desc);
        ~VulkanShader() override;

        VulkanShader(const VulkanShader& other) = delete;
        VulkanShader(VulkanShader&& other) noexcept = default;
        VulkanShader& operator=(const VulkanShader& other) = delete;
        VulkanShader& operator=(VulkanShader&& other) noexcept = default;

        //--------------------------------------------------
		//    Accessors
		//--------------------------------------------------
        ShaderStage GetStage() const override;

        //--------------------------------------------------
		//    Operator Overloads
		//--------------------------------------------------
		operator VkShaderModule() const;

    private:
	    static std::vector<char> ReadShader(const std::filesystem::path& filePath);

	    VulkanContext* m_pContext   { nullptr };

        VkShaderModule m_Module     { VK_NULL_HANDLE };
        ShaderStage m_Stage;
    };
}

#endif // POMPEII_VULKAN_SHADER_H
