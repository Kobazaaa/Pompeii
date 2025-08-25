#ifndef GENERAL_LIGHT_H
#define GENERAL_LIGHT_H

// -- Math Includes --
#include "glm/glm.hpp"

// -- Pompeii Includes --
#include "Component.h"
#include "Light.h"

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
	//? ~~	  Light Component	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class LightComponent final : public Component
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		LightComponent(SceneObject& parent, const glm::vec3& dirPos, const glm::vec3& col, float luxLumen, LightType type);
		~LightComponent() override;

		LightComponent(const LightComponent& other) = delete;
		LightComponent(LightComponent&& other) noexcept = delete;
		LightComponent& operator=(const LightComponent& other) = delete;
		LightComponent& operator=(LightComponent&& other) noexcept = delete;


		//--------------------------------------------------
		//    Loop
		//--------------------------------------------------
		void Start() override;
		void Update() override;
		void OnInspectorDraw() override;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		Light lightData{};
	};
}

#endif // GENERAL_LIGHT_H