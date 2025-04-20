// _- Math Includes --
#include <glm/gtc/matrix_transform.hpp>

// -- Pompeii Includes --
#include "DirectionalLight.h"


//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::DirectionalLight::DirectionalLight(const glm::vec3& dir, const glm::vec3& col, float intensity, const glm::vec2& size, const glm::vec2& nearFar, float distance)
	: m_Direction(normalize(dir))
	, m_Color(col)
	, m_Intensity(intensity)
	, m_Size(size)
	, m_NearFarPlane(nearFar)
	, m_Distance(distance)
{}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
glm::vec3 pom::DirectionalLight::GetDirection()					const	{	return m_Direction; }
void pom::DirectionalLight::SetDirection(const glm::vec3& dir)			{ m_IsDirty = true; m_Direction = normalize(dir); }

glm::vec3 pom::DirectionalLight::GetColor()						const	{	return m_Color; }
void pom::DirectionalLight::SetColor(const glm::vec3& col)				{ m_Color = col; }

float pom::DirectionalLight::GetIntensity()						const	{	return m_Intensity; }
void pom::DirectionalLight::SetIntensity(float intensity)				{ m_Intensity = intensity; }

glm::mat4 pom::DirectionalLight::GetLightSpaceMatrix()
{
	if (m_IsDirty)
	{
		const float halfSizeX = m_Size.x / 2.f;
		const float halfSizeY = m_Size.y / 2.f;

		const glm::mat4 proj = glm::orthoLH(
			-halfSizeX, halfSizeX,
			-halfSizeY, halfSizeY,
			m_NearFarPlane.x, m_NearFarPlane.y
		);

		const glm::mat4 view = glm::lookAtLH(-m_Direction * m_Distance, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
		m_LightSpaceMatrix = proj * view;
	}
    return m_LightSpaceMatrix;
}
