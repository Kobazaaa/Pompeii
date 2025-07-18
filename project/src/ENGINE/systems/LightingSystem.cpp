// -- Pompeii Includes --
#include "Transform.h"
#include "LightingSystem.h"
#include "ModelRenderer.h"
#include "LightComponent.h"

// -- Standard Library --
#include <numeric>

//--------------------------------------------------
//    Lights
//--------------------------------------------------
void pompeii::LightingSystem::RegisterLight(LightComponent& light)
{
	if (std::ranges::find(m_vRegisteredLights, &light) != m_vRegisteredLights.end())
		return;
	m_vRegisteredLights.emplace_back(&light);
	auto handle = m_pRenderer->CreateLight(light.lightData);
	light.m_LightHandle = handle;
	m_pRenderer->UpdateLights();

}
void pompeii::LightingSystem::UnregisterLight(const LightComponent& light)
{
	std::erase_if(m_vRegisteredLights, [&](const LightComponent* pLight)
	{
		if (pLight == &light)
		{
			m_pRenderer->DestroyLight(light.GetLightHandle());
			return true;
		}
		return false;
	});
}

//std::vector<pompeii::GPULight> pompeii::RenderSystem::GetLightsGPU() const
//{
//	std::vector<pompeii::GPULight> res{};
//	res.reserve(m_vRegisteredLights.size());
//
//	uint32_t matrixIdx = 0;
//	uint32_t directionalCounter = 0;
//	uint32_t pointCounter = 0;
//	for (auto& l : m_vRegisteredLights)
//	{
//		GPULight gpuL{};
//		LightComponent::Type type = l->GetType();
//
//		gpuL.dirPosType = { type == LightComponent::Type::Point ? l->dirPos : normalize(l->dirPos), static_cast<int>(l->GetType()) };
//		gpuL.intensity = l->luxLumen;
//		gpuL.color = l->color;
//		gpuL.matrixIndex = 0xFFFFFFFF;
//		gpuL.depthIndex = 0xFFFFFFFF;
//
//		switch (type)
//		{
//		case LightComponent::Type::Point:
//			gpuL.depthIndex = pointCounter;
//			++pointCounter;
//			break;
//		case LightComponent::Type::Directional:
//			gpuL.matrixIndex = matrixIdx;
//			gpuL.depthIndex = directionalCounter;
//			++directionalCounter;
//			++matrixIdx;
//			break;
//		}
//
//		res.push_back(std::move(gpuL));
//	}
//	return res;
//}
//std::vector<glm::mat4> pompeii::RenderSystem::GetLightMatrices() const
//{
//	std::vector<glm::mat4> res{};
//	for (auto& l : m_vRegisteredLights)
//	{
//		if (l->GetType() == LightComponent::Type::Directional)
//		{
//			const auto& proj = l->projMatrix;
//			const auto& view = l->viewMatrices.front();
//			res.push_back(proj * view);
//		}
//	}
//	return res;
//}

//--------------------------------------------------
//    Interface
//--------------------------------------------------
void pompeii::LightingSystem::Update()
{
}
void pompeii::LightingSystem::BeginFrame()
{
}
void pompeii::LightingSystem::EndFrame()
{
}

void pompeii::LightingSystem::SetRenderer(const std::shared_ptr<Renderer>& renderer)
{
	m_pRenderer = renderer;
}
pompeii::Renderer* pompeii::LightingSystem::GetRenderer() const
{
	return m_pRenderer.get();
}


