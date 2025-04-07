#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>
#include "Device.h"

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  ShaderModule	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class ShaderModule final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		ShaderModule() = default;
		void Destroy(const Device& device) const;


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkShaderModule& GetShader() const;

	private:
		VkShaderModule m_Shader;
		friend class ShaderLoader;
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  ShaderLoader	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class ShaderLoader final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		ShaderLoader() = default;


		//--------------------------------------------------
		//    Loader
		//--------------------------------------------------
		void Load(const Device& device, const std::string& filename, ShaderModule& module);

	private:
		void ReadCode(const std::string& filename);
		std::vector<char> m_vCode{};
	};
}

#endif // SHADER_H