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

pom::ShaderModule pom::ShaderLoader::Load(const Device& device, const std::string& filename)
{
	ReadCode(filename);
	return BuildModule(device);
}

std::vector<pom::ShaderModule> pom::ShaderLoader::LoadMultiple(const Device& device, const std::initializer_list<std::string>& fileNames, const std::string& prefix)
{
	std::vector<ShaderModule> shaders;
	shaders.reserve(fileNames.size());
	for (auto& fileName : fileNames)
		shaders.emplace_back(Load(device, prefix + fileName));
	return shaders;
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
pom::ShaderModule pom::ShaderLoader::BuildModule(const Device& device)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = m_vCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(m_vCode.data());

	ShaderModule shaderModule{};
	if (vkCreateShaderModule(device.GetDevice(), &createInfo, nullptr, &shaderModule.m_Shader) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Shader Module!");

	m_vCode.clear();
	return shaderModule;
}
