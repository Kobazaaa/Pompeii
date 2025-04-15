#ifndef MATERIAL_TEXTURE_H
#define MATERIAL_TEXTURE_H

// -- Standard Library --
#include <string>

// -- Texture Includes --
#include "stb_image.h"

// -- Math Includes --
#include "glm/vec2.hpp"


namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Texture	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Texture final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		Texture() = default;
		void LoadFromFile(const std::string& path);
		void FreePixels() const;


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		stbi_uc* GetPixels() const;
		uint32_t GetMemorySize() const;
		glm::ivec2 GetExtent() const;

	private:
		stbi_uc* m_pPixels;
		int m_Width;
		int m_Height;
		int m_Channels;
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Material	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct Material
	{
		// -- Diffuse --
		uint32_t diffuseIdx;

		// -- Opacity --
		uint32_t opacityIdx;
	};
}

#endif // MATERIAL_TEXTURE_H