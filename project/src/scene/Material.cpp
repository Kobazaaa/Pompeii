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
pom::Texture::Texture(const std::string& path, VkFormat format, bool incIdx)
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
pom::Texture::~Texture()
{
	FreePixels();
}
pom::Texture::Texture(Texture&& other) noexcept
{
	m_pPixels = other.m_pPixels;
	other.m_pPixels = nullptr;
	m_Width = other.m_Width;
	m_Height = other.m_Height;
	m_Channels = other.m_Channels;
	m_Format = other.m_Format;
	other.m_Format = VK_FORMAT_UNDEFINED;
	m_Index = other.m_Index;
}
pom::Texture& pom::Texture::operator=(Texture&& other) noexcept
{
	if (this == &other)
		return *this;
	m_pPixels = other.m_pPixels;
	other.m_pPixels = nullptr;
	m_Width = other.m_Width;
	m_Height = other.m_Height;
	m_Channels = other.m_Channels;
	m_Format = other.m_Format;
	other.m_Format = VK_FORMAT_UNDEFINED;
	m_Index = other.m_Index;
	return *this;
}

void pom::Texture::FreePixels()
{
	if (m_pPixels)
	{
		stbi_image_free(m_pPixels);
		m_pPixels = nullptr;
	}
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
