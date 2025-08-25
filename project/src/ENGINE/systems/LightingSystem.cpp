// -- Pompeii Includes --
#include "LightingSystem.h"
#include "LightComponent.h"

// -- Standard Library --
#include <iterator>

#include "Scene.h"
#include "SceneObject.h"

//--------------------------------------------------
//    Lights
//--------------------------------------------------
void pompeii::LightingSystem::RegisterLight(LightComponent& light)
{
	auto it = std::ranges::find(m_vRegisteredLights, &light);
	if (it != m_vRegisteredLights.end())
		return;
	it = std::ranges::find(m_vPendingLights, &light);
	if (it != m_vPendingLights.end())
		return;
	m_UpdateLights = true;

	light.lightData.CalculateLightMatrices(light.GetSceneObject().GetScene().GetAABB());
	light.lightData.CreateDepthImage(m_pRenderer->GetContext(), 4096);

	m_vPendingLights.emplace_back(&light);
}
void pompeii::LightingSystem::UnregisterLight(LightComponent& light)
{
	m_UpdateLights = true;

	std::erase_if(m_vRegisteredLights, [&](const LightComponent* pLight)
	{
		if (pLight == &light)
		{
			m_pRenderer->GetContext().device.WaitIdle();
			light.lightData.DestroyDepthMap(m_pRenderer->GetContext());
		}
		return pLight == &light;
	});
	std::erase_if(m_vPendingLights, [&](const LightComponent* pLight)
	{
		if (pLight == &light)
		{
			m_pRenderer->GetContext().device.WaitIdle();
			light.lightData.DestroyDepthMap(m_pRenderer->GetContext());
		}
		return pLight == &light;
	});
}

void pompeii::LightingSystem::UpdateLight(LightComponent&)
{
	m_UpdateLights = true;
}

//--------------------------------------------------
//    Interface
//--------------------------------------------------
void pompeii::LightingSystem::Update()
{
	for (LightComponent* l : m_vRegisteredLights)
	{
		m_pRenderer->SubmitLightItem(LightItem
			{
				.light = &l->lightData
			});
	}
	UpdateData();
}
void pompeii::LightingSystem::BeginFrame()
{
	AddPendingObjects();
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


//--------------------------------------------------
//    Helpers
//--------------------------------------------------
void pompeii::LightingSystem::AddPendingObjects()
{
	if (!m_vPendingLights.empty())
	{
		for (auto& pendingLight : m_vPendingLights)
			m_vRegisteredLights.emplace_back(pendingLight);
		m_vPendingLights.clear();
	}
}
void pompeii::LightingSystem::UpdateData()
{
	if (m_UpdateLights)
	{
		m_UpdateLights = false;

		std::vector<Light*> newLights{};
		newLights.reserve(m_vRegisteredLights.size());
		for (auto& registeredLight : m_vRegisteredLights)
			newLights.push_back(&registeredLight->lightData);
		m_pRenderer->UpdateLights(newLights);
	}
}