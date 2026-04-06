// -- Pompeii Includes --
#include "VulkanShader.h"
#include "VulkanContext.h"

// -- Standard Library --
#include <fstream>


using namespace pompeii;
using namespace pompeii::vulkan;

VulkanShader::VulkanShader(VulkanContext& context, const ShaderDesc& desc)
    : m_pContext(&context)
    , m_Stage(desc.stage)
{
    const auto code = ReadShader(desc.path);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(m_pContext->device, &createInfo, nullptr, &m_Module) != VK_SUCCESS)
        throw std::runtime_error("Failed to create shader module!");
}

VulkanShader::~VulkanShader()
{
    if (m_Module) vkDestroyShaderModule(m_pContext->device, m_Module, nullptr);
}


//--------------------------------------------------
//    Accessors
//--------------------------------------------------
ShaderStage VulkanShader::GetStage() const { return m_Stage; }

//--------------------------------------------------
//    Operator Overloads
//--------------------------------------------------
VulkanShader::operator VkShaderModule() const { return m_Module; }


//--------------------------------------------------
//    Helper
//--------------------------------------------------
std::vector<char> VulkanShader::ReadShader(const std::filesystem::path& filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file: " + filePath.string());

    const size_t fileSize = (size_t)file.tellg();

    std::vector<char> code{};
    code.resize(fileSize);

    file.seekg(0);
    file.read(code.data(), static_cast<uint32_t>(fileSize));

    file.close();

    return code;
}
