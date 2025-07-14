// -- Standard Library --
#include <stdexcept>

// -- Pompeii Includes --
#include "Material.h"

// -- Texture Includes --
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Texture	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::Texture::Texture(const std::string& path, VkFormat format, bool isHDR)
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
pompeii::Texture::~Texture()
{
	FreePixels();
}
pompeii::Texture::Texture(Texture&& other) noexcept
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
pompeii::Texture& pompeii::Texture::operator=(Texture&& other) noexcept
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

void pompeii::Texture::FreePixels()
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
void* pompeii::Texture::GetPixels()			const {	return m_pPixels; }
uint32_t pompeii::Texture::GetMemorySize()	const
{
	const int pixelSize = (m_DataType == TextureDataType::FLOAT32) ? sizeof(float) : sizeof(stbi_uc);
	return m_Width * m_Height * m_Channels * pixelSize;
}
glm::ivec2 pompeii::Texture::GetExtent()		const { return {m_Width, m_Height}; }
VkFormat pompeii::Texture::GetFormat()			const { return m_Format; }
const std::string& pompeii::Texture::GetPath()	const { return m_Path; }
