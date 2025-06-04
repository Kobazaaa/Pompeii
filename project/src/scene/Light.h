#ifndef GENERAL_LIGHT_H
#define GENERAL_LIGHT_H

// -- Standard Library --
#include <vector>

// -- Math Includes --
#include "glm/glm.hpp"

// -- Pompeii Includes --
#include "Image.h"

// -- Forward Declarations --
namespace pom
{
	struct AABB;
	struct Context;
	class Scene;
}

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  GPU Representation Light	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct alignas(16) GPULight
	{
		glm::vec4 dirPosType;
		glm::vec3 color;
		float intensity;
		uint32_t shadowMatrixOffset;
		uint32_t shadowMatrixCount;
		float _padding[2];
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Light	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Light final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		enum class Type
		{
			Directional, Point
		};
		Light() = default;
		Light(const glm::vec3& dirPos, const glm::vec3& col, float luxLumen, Type type);
		void DestroyDepthMap(const Context& context);

		Light(const Light& other) = delete;
		Light(Light&& other) noexcept;
		Light& operator=(const Light& other) = delete;
		Light& operator=(Light&& other) noexcept;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		// -- Data --
		Type GetType() const;
		glm::vec3 dirPos;
		glm::vec3 color;
		float luxLumen;

		// -- Matrices --
		std::vector<glm::mat4> viewMatrices;
		glm::mat4 projMatrix;

		const Image& GetDepthMap() const;
		void GenerateDepthMap(const Context& context, const Scene* pScene, uint32_t size = 2048);

	private:
		// -- Helper --
		void GenerateDepthMap(const Context& context, const Scene* pScene, Image& outImage, std::vector<ImageView>& outViews, uint32_t size);

		// -- Data --
		Type m_Type;

		// -- Depth Map --
		Image m_DepthMap;
	};
}

#endif // GENERAL_LIGHT_H