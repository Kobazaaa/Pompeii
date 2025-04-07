#include "Shader.h"

#include <fstream>
#include <stdexcept>


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  ShaderModule	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::ShaderModule::Destroy(const Device& device) const { vkDestroyShaderModule(device.GetDevice(), m_Shader, nullptr); }


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkShaderModule& pom::ShaderModule::GetShader()  const { return m_Shader; }


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  ShaderLoader	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Loader
//--------------------------------------------------

void pom::ShaderLoader::Load(const Device& device, const std::string& filename, ShaderModule& module)
{
	ReadCode(filename);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = m_vCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(m_vCode.data());

	if (vkCreateShaderModule(device.GetDevice(), &createInfo, nullptr, &module.m_Shader) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Shader Module!");

	m_vCode.clear();
}

void pom::ShaderLoader::ReadCode(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("Failed to open file!");

	size_t fileSize = (size_t)file.tellg();

	m_vCode.clear();
	m_vCode.resize(fileSize);

	file.seekg(0);
	file.read(m_vCode.data(), static_cast<uint32_t>(fileSize));

	file.close();
}
