// _- Math Includes --
#include <glm/gtc/matrix_transform.hpp>

// -- Pompeii Includes --
#include "Light.h"
#include "Model.h"


//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::Light::Light(const glm::vec3& dirPos, const glm::vec3& col, float luxLumen, Type type)
	: m_Type(type)
	, m_DirPos(type == Type::Directional ? glm::normalize(dirPos) : dirPos)
	, m_Color(col)
	, m_LuxLumen(luxLumen)
{ }


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
pom::Light::Type pom::Light::GetType()				const	{ return m_Type; }

glm::vec3 pom::Light::GetDirPos()					const	{ return m_DirPos; }
void pom::Light::SetDirPos(const glm::vec3& dirPos)			{ m_DirPos = m_Type == Type::Directional ? glm::normalize(dirPos) : dirPos; }

glm::vec3 pom::Light::GetColor()					const	{ return m_Color; }
void pom::Light::SetColor(const glm::vec3& col)				{ m_Color = col; }

float pom::Light::GetLuxLumen()						const	{ return m_LuxLumen; }
void pom::Light::SetLuxLumen(float luxLumen)				{ m_LuxLumen = luxLumen; }

void pom::Light::CalculateMatrices(const AABB& aabb)
{
	// -- Directional --
	if (m_Type == Type::Directional)
	{
		const glm::vec3 center = (aabb.min + aabb.max) * 0.5f;
		const glm::vec3 l = m_DirPos;

		const std::vector<glm::vec3> corners = {
			{aabb.min.x, aabb.min.y, aabb.min.z},
			{aabb.max.x, aabb.min.y, aabb.min.z},
			{aabb.min.x, aabb.max.y, aabb.min.z},
			{aabb.max.x, aabb.max.y, aabb.min.z},
			{aabb.min.x, aabb.min.y, aabb.max.z},
			{aabb.max.x, aabb.min.y, aabb.max.z},
			{aabb.min.x, aabb.max.y, aabb.max.z},
			{aabb.max.x, aabb.max.y, aabb.max.z}
		};

		float minProj = FLT_MAX;
		float maxProj = -FLT_MAX;
		for (const auto& c : corners)
		{
			const float proj = glm::dot(c, l);
			minProj = std::min(minProj, proj);
			maxProj = std::max(maxProj, proj);
		}

		const float dst = maxProj - glm::dot(center, l);
		const glm::vec3 lightPos = center - l * dst;

		const glm::vec3 up = glm::abs(glm::dot(l, glm::vec3(0.f, 1.f, 0.f))) < (1.f - FLT_EPSILON) ?
							 glm::vec3(0.f, 1.f, 0.f) : glm::vec3(0.f, 0.f, -1.f);
		auto lookAt = glm::lookAtLH(lightPos, center, up);
		m_ViewMatrix.resize(1);
		m_ViewMatrix[0] = lookAt;

		glm::vec3 minLightSpace(FLT_MAX);
		glm::vec3 maxLightSpace(-FLT_MAX);
		for (const auto& c : corners)
		{
			const glm::vec3 transformedCorner = glm::vec3(lookAt * glm::vec4(c, 1.f));
			minLightSpace = glm::min(minLightSpace, transformedCorner);
			maxLightSpace = glm::max(maxLightSpace, transformedCorner);
		}

		constexpr float nearZ = 0.f;
		const float farZ = maxLightSpace.z - minLightSpace.z;
		m_ProjMatrix = glm::orthoLH(minLightSpace.x, maxLightSpace.x, minLightSpace.y, maxLightSpace.y, nearZ, farZ);
	}
	// -- Point --
	{

	}
}

const std::vector<glm::mat4>& pom::Light::GetViewMatrices()		const { return m_ViewMatrix; }
const glm::mat4& pom::Light::GetProjectionMatrix()				const { return m_ProjMatrix; }
