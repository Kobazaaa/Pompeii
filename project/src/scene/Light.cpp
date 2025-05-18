// _- Math Includes --
#include <glm/gtc/matrix_transform.hpp>

// -- Pompeii Includes --
#include "Light.h"


//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::Light::Light(const glm::vec3& dirPos, const glm::vec3& col, float intensity, Type type)
	: m_Type(type)
	, m_DirPos(type == Type::Directional ? glm::normalize(dirPos) : dirPos)
	, m_Color(col)
	, m_Intensity(intensity)
{
	m_GPULight =
	{
		.dirPosType = {m_DirPos, static_cast<int>(m_Type)},
		.color = m_Color,
		.intensity = m_Intensity
	};
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const pom::GPULight& pom::Light::GetGPULight()		const	{ return m_GPULight; }
pom::Light::Type pom::Light::GetType()				const	{ return m_Type; }

glm::vec3 pom::Light::GetDirPos()					const	{ return m_DirPos; }
void pom::Light::SetDirPos(const glm::vec3& dirPos)			{ m_DirPos = m_Type == Type::Directional ? glm::normalize(dirPos) : dirPos; m_GPULight.dirPosType = { m_DirPos, m_GPULight.dirPosType.w }; }

glm::vec3 pom::Light::GetColor()					const	{ return m_Color; }
void pom::Light::SetColor(const glm::vec3& col)				{ m_Color = col; m_GPULight.color = m_Color;}

float pom::Light::GetIntensity()					const	{ return m_Intensity; }
void pom::Light::SetIntensity(float intensity)				{ m_Intensity = intensity; m_GPULight.intensity = m_Intensity;}
