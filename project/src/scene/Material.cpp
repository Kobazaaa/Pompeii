// -- Standard Library --
#include <stdexcept>

// -- Pompeii Includes --
#include "Material.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Texture	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::Texture::LoadFromFile(const std::string& path, VkFormat format, bool incIdx)
{
	m_Index = 0xFFFFFFFF;
	if (incIdx)
	{
		m_Index = index;
		++index;
	}
	m_Format = format;
	m_pPixels = stbi_load(path.c_str(), &m_Width, &m_Height, &m_Channels, STBI_rgb_alpha);
	if (!m_pPixels)
		throw std::runtime_error("Failed to load Texture: " + path);
}
void pom::Texture::FreePixels() const
{
	stbi_image_free(m_pPixels);
}

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
stbi_uc* pom::Texture::GetPixels()		const
{
	return m_pPixels;
}
uint32_t pom::Texture::GetMemorySize()	const { return m_Width * m_Height * 4; }
glm::ivec2 pom::Texture::GetExtent()	const { return {m_Width, m_Height}; }
VkFormat pom::Texture::GetFormat()		const { return m_Format; }
uint32_t pom::Texture::GetLocalIndex()  const { return m_Index; }
uint32_t pom::Texture::GetStaticIndex()		  { return index; }
