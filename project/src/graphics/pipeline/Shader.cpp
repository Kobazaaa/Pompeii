// -- Standard Library --
#include <fstream>
#include <stdexcept>

// -- Pompeii Includes --
#include "Shader.h"
#include "Context.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  ShaderModule	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::ShaderModule::Destroy(const Context& context) const { vkDestroyShaderModule(context.device.GetHandle(), m_Shader, nullptr); }


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkShaderModule& pom::ShaderModule::GetHandle()  const { return m_Shader; }


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  ShaderLoader	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Loader
//--------------------------------------------------

void pom::ShaderLoader::Load(const Context& context, const std::string& filename, ShaderModule& module)
{
	ReadCode(filename);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = m_vCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(m_vCode.data());

	if (vkCreateShaderModule(context.device.GetHandle(), &createInfo, nullptr, &module.m_Shader) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Shader Module!");

	m_vCode.clear();
}

void pom::ShaderLoader::ReadCode(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("Failed to open file: " + filename);

	size_t fileSize = (size_t)file.tellg();

	m_vCode.clear();
	m_vCode.resize(fileSize);

	file.seekg(0);
	file.read(m_vCode.data(), static_cast<uint32_t>(fileSize));

	file.close();
}
