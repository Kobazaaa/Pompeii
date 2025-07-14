#ifndef MATERIAL_TEXTURE_H
#define MATERIAL_TEXTURE_H

// -- Vulkan --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <string>

// -- Math Includes --
#include "glm/glm.hpp"


namespace pompeii
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
		explicit Texture(const std::string& path, VkFormat format, bool isHDR = false);
		~Texture();
		Texture(const Texture& other) = delete;
		Texture(Texture&& other) noexcept;
		Texture& operator=(const Texture& other) = delete;
		Texture& operator=(Texture&& other) noexcept;
		void FreePixels();

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		void* GetPixels() const;
		uint32_t GetMemorySize() const;
		glm::ivec2 GetExtent() const;
		VkFormat GetFormat() const;
		const std::string& GetPath() const;

	private:
		enum class TextureDataType { UINT8, FLOAT32 };
		TextureDataType m_DataType;

		void* m_pPixels;
		int m_Width;
		int m_Height;
		int m_Channels;
		VkFormat m_Format;

		std::string m_Path{};
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Material	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct Material
	{
		// -- Colors --
		glm::vec4 baseColor		{ };

		// -- Data --
		glm::vec3 kd			{ 0.f, 0.f, 0.f };
		bool metalness			{ false };
		bool _padding[3];
		float roughness			{ 0.f };
		float alphaCutoff		{ 0.5f };

		// -- Textures --
		uint32_t albedoIdx		{ std::numeric_limits<uint32_t>::max() };
		uint32_t normalIdx		{ std::numeric_limits<uint32_t>::max() };
		uint32_t metalnessIdx	{ std::numeric_limits<uint32_t>::max() };
		uint32_t roughnessIdx	{ std::numeric_limits<uint32_t>::max() };

		uint32_t opacityIdx		{ std::numeric_limits<uint32_t>::max() };
		uint32_t specularIdx	{ std::numeric_limits<uint32_t>::max() };
		uint32_t shininessIdx	{ std::numeric_limits<uint32_t>::max() };
		uint32_t heightIdx		{ std::numeric_limits<uint32_t>::max() };
	};
}

#endif // MATERIAL_TEXTURE_H