#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

// -- Math Includes --
#include "glm/glm.hpp"

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Directional Light	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class DirectionalLight
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		DirectionalLight() = default;
		DirectionalLight(const glm::vec3& dir, const glm::vec3& col, float intensity);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		glm::vec3 GetDirection() const;
		void SetDirection(const glm::vec3& dir);

		glm::vec3 GetColor() const;
		void SetColor(const glm::vec3& col);

		float GetIntensity() const;
		void SetIntensity(float intensity);

		glm::mat4 GetLightSpaceMatrix();

	private:
		// -- Data --
		glm::vec3 m_Direction;
		glm::vec3 m_Color;
		float m_Intensity;

		// -- Matrix --
		glm::mat4 m_LightSpaceMatrix;
		glm::vec2 m_Size;
		glm::vec2 m_NearFarPlane;

		// -- Help --
		float m_Distance;
		bool m_IsDirty{ true };
	};
}

#endif // DIRECTIONAL_LIGHT_H