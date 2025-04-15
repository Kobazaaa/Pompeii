#ifndef SHADER_H
#define SHADER_H

// -- Vulkan Includes --
#include "vulkan/vulkan.h"

// --  Standard Library --
#include <string>
#include <vector>

// -- Forward Declarations --
namespace pom { struct Context; }


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
		void Destroy(const Context& context) const;


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkShaderModule& GetHandle() const;

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
		void Load(const Context& context, const std::string& filename, ShaderModule& module);

	private:
		void ReadCode(const std::string& filename);
		std::vector<char> m_vCode{};
	};
}

#endif // SHADER_H