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
pom::Texture::Texture(const std::string& path, VkFormat format, bool isHDR)
{
	m_Format = format;
	m_Path = path;

	m_pPixels = isHDR ? static_cast<void*>(stbi_loadf(path.c_str(), &m_Width, &m_Height, &m_Channels, STBI_rgb_alpha)) :
						static_cast<void*>(stbi_load(path.c_str(), &m_Width, &m_Height, &m_Channels, STBI_rgb_alpha));
	m_DataType = isHDR ? TextureDataType::FLOAT32 : TextureDataType::UINT8;
	m_Channels = 4;
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
	m_DataType = other.m_DataType;
	m_Width = other.m_Width;
	m_Height = other.m_Height;
	m_Channels = other.m_Channels;
	m_Format = other.m_Format;
	other.m_Format = VK_FORMAT_UNDEFINED;
}
pom::Texture& pom::Texture::operator=(Texture&& other) noexcept
{
	if (this == &other)
		return *this;
	m_pPixels = other.m_pPixels;
	other.m_pPixels = nullptr;
	m_DataType = other.m_DataType;
	m_Width = other.m_Width;
	m_Height = other.m_Height;
	m_Channels = other.m_Channels;
	m_Format = other.m_Format;
	other.m_Format = VK_FORMAT_UNDEFINED;
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
void* pom::Texture::GetPixels()			const {	return m_pPixels; }
uint32_t pom::Texture::GetMemorySize()	const
{
	const int pixelSize = (m_DataType == TextureDataType::FLOAT32) ? sizeof(float) : sizeof(stbi_uc);
	return m_Width * m_Height * m_Channels * pixelSize;
}
glm::ivec2 pom::Texture::GetExtent()		const { return {m_Width, m_Height}; }
VkFormat pom::Texture::GetFormat()			const { return m_Format; }
const std::string& pom::Texture::GetPath()	const { return m_Path; }
