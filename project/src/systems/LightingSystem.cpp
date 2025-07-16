// -- Pompeii Includes --
#include "LightingSystem.h"
#include "Light.h"

// -- Standard Library --
#include <numeric>

//--------------------------------------------------
//    Lights
//--------------------------------------------------
void pompeii::LightingSystem::RegisterLight(Light& light)
{
	if (std::ranges::find(m_vRegisteredLights, &light) != m_vRegisteredLights.end())
		return;
	m_vRegisteredLights.emplace_back(&light);
}
void pompeii::LightingSystem::UnregisterLight(const Light& light)
{
	std::erase_if(m_vRegisteredLights, [&](const Light* pLight) { return pLight == &light; });
}
const std::vector<pompeii::Light*>& pompeii::LightingSystem::GetLights() const
{
	return m_vRegisteredLights;
}

std::vector<pompeii::GPULight> pompeii::LightingSystem::GetLightsGPU() const
{
	std::vector<pompeii::GPULight> res{};
	res.reserve(m_vRegisteredLights.size());

	uint32_t matrixIdx = 0;
	uint32_t directionalCounter = 0;
	uint32_t pointCounter = 0;
	for (auto& l : m_vRegisteredLights)
	{
		GPULight gpuL{};
		Light::Type type = l->GetType();

		gpuL.dirPosType = { type == Light::Type::Point ? l->dirPos : normalize(l->dirPos), static_cast<int>(l->GetType()) };
		gpuL.intensity = l->luxLumen;
		gpuL.color = l->color;
		gpuL.matrixIndex = 0xFFFFFFFF;
		gpuL.depthIndex = 0xFFFFFFFF;

		switch (type)
		{
		case Light::Type::Point:
			gpuL.depthIndex = pointCounter;
			++pointCounter;
			break;
		case Light::Type::Directional:
			gpuL.matrixIndex = matrixIdx;
			gpuL.depthIndex = directionalCounter;
			++directionalCounter;
			++matrixIdx;
			break;
		}

		res.push_back(std::move(gpuL));
	}
	return res;
}
uint32_t pompeii::LightingSystem::GetLightsCount() const
{
	return static_cast<uint32_t>(m_vRegisteredLights.size());
}
std::vector<glm::mat4> pompeii::LightingSystem::GetLightMatrices() const
{
	std::vector<glm::mat4> res{};
	for (auto& l : m_vRegisteredLights)
	{
		if (l->GetType() == Light::Type::Directional)
		{
			const auto& proj = l->projMatrix;
			const auto& view = l->viewMatrices.front();
			res.push_back(proj * view);
		}
	}
	return res;
}

//--------------------------------------------------
//    Interface
//--------------------------------------------------
