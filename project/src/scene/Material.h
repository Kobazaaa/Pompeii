#ifndef MATERIAL_TEXTURE_H
#define MATERIAL_TEXTURE_H

// -- Standard Library --
#include <string>

// -- Texture Includes --
#include "stb_image.h"

// -- Math Includes --
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"


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
		// -- Textures --
		uint32_t diffuseIdx		{ std::numeric_limits<uint32_t>::max() };
		uint32_t opacityIdx		{ std::numeric_limits<uint32_t>::max() };
		uint32_t specularIdx	{ std::numeric_limits<uint32_t>::max() };
		uint32_t shininessIdx	{ std::numeric_limits<uint32_t>::max() };
		uint32_t heightIdx		{ std::numeric_limits<uint32_t>::max() };

		// -- Data --
		float exp				{ 0.f };
	};
}

#endif // MATERIAL_TEXTURE_H