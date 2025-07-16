#ifndef GENERAL_LIGHT_H
#define GENERAL_LIGHT_H

// -- Standard Library --
#include <vector>

// -- Math Includes --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

// -- Pompeii Includes --
#include "Component.h"
#include "Image.h"

// -- Forward Declarations --
namespace pompeii
{
	struct AABB;
	struct Context;
	class Scene;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  GPU Representation Light	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct alignas(16) GPULight
	{
		glm::vec4 dirPosType;
		glm::vec3 color;
		float intensity;
		uint32_t matrixIndex;
		uint32_t depthIndex;
		float _padding[2];
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Light	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Light final : public Component
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		enum class Type
		{
			Directional, Point
		};
		Light(SceneObject& parent, const glm::vec3& dirPos, const glm::vec3& col, float luxLumen, Type type);
		~Light() override;

		Light(const Light& other) = delete;
		Light(Light&& other) noexcept = delete;
		Light& operator=(const Light& other) = delete;
		Light& operator=(Light&& other) noexcept = delete;


		//--------------------------------------------------
		//    Loop
		//--------------------------------------------------
		void Start() override;
		void OnImGuiRender() override;

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
		void DestroyDepthMap(const Context& context);
		void CalculateLightMatrices();

	private:
		// -- Helper --
		void GenerateDepthMap(const Context& context, const Scene* pScene, Image& outImage, std::vector<ImageView>& outViews, uint32_t size) const;

		// -- Data --
		Type m_Type;

		// -- Depth Map --
		Image m_DepthMap;
	};
}

#endif // GENERAL_LIGHT_H