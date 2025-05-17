#ifndef MATERIAL_TEXTURE_H
#define MATERIAL_TEXTURE_H

// -- Vulkan --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <string>

// -- Texture Includes --
#include "stb_image.h"

// -- Math Includes --
#include "glm/glm.hpp"


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
		void LoadFromFile(const std::string& path, VkFormat format, bool incIdx = true);
		void FreePixels() const;


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		stbi_uc* GetPixels() const;
		uint32_t GetMemorySize() const;
		glm::ivec2 GetExtent() const;
		VkFormat GetFormat() const;
		uint32_t GetLocalIndex() const;
		static uint32_t GetStaticIndex();

	private:
		stbi_uc* m_pPixels;
		int m_Width;
		int m_Height;
		int m_Channels;
		VkFormat m_Format;
		uint32_t m_Index{};

		inline static uint32_t index{0};
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