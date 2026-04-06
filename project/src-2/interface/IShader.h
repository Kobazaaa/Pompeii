#ifndef POMPEII_SHADER_INTERFACE_H
#define POMPEII_SHADER_INTERFACE_H

// -- Standard Library --
#include <filesystem>

// -- Pompeii Includes --
#include "PompeiiEnums.h"

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    Shader Description
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct ShaderDesc
	{
		std::filesystem::path path;
		ShaderStage stage;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    IShader
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class IShader
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		virtual ~IShader() = default;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		virtual ShaderStage GetStage() const = 0;
	};
}

#endif // POMPEII_SHADER_INTERFACE_H
