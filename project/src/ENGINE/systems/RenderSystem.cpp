// -- Pompeii Includes --
#include "RenderSystem.h"
#include "MeshRenderer.h"
#include "Camera.h"

// -- Standard Library --
#include <numeric>

//--------------------------------------------------
//    Models
//--------------------------------------------------
void pompeii::RenderSystem::RegisterMeshRenderer(MeshRenderer& model)
{
	auto it = std::ranges::find(m_vRegisteredModels, &model);
	if (it != m_vRegisteredModels.end())
		return;
	it = std::ranges::find(m_vPendingModels, &model);
	if (it != m_vPendingModels.end())
		return;

	m_vPendingModels.emplace_back(&model);
	m_UpdateModels = true;
}
void pompeii::RenderSystem::UnregisterMeshRenderer(const MeshRenderer& model)
{
	std::erase_if(m_vRegisteredModels, [&](const MeshRenderer* pModel)
	{
		return pModel == &model;
	});
	std::erase_if(m_vPendingModels, [&](const MeshRenderer* pModel)
	{
		return pModel == &model;
	});
	std::erase_if(m_vVisibleModels, [&](const MeshRenderer* pModel)
	{
		return pModel == &model;
	});
	m_UpdateModels = true;
}


//--------------------------------------------------
//    Camera
//--------------------------------------------------
pompeii::Camera* pompeii::RenderSystem::GetMainCamera() const
{
	return m_pMainCamera;
}
void pompeii::RenderSystem::SetMainCamera(Camera& camera)
{
	m_pMainCamera = &camera;
}

//--------------------------------------------------
//    Interface
//--------------------------------------------------
void pompeii::RenderSystem::Update()
{
	for (auto& visibleModel : m_vVisibleModels)
	{
		if (visibleModel->pMeshFilter)
			m_pRenderer->SubmitRenderItem(RenderItem
				{
					.mesh = visibleModel->pMeshFilter->pMesh,
					.transform = visibleModel->GetTransform().GetMatrix()
				});
	}
	m_pRenderer->SetCamera(CameraData{
			.view = m_pMainCamera->GetViewMatrix(),
			.proj = m_pMainCamera->GetProjectionMatrix(),
			.manualExposureSettings = m_pMainCamera->GetManualExposureSettings(),
			.autoExposureSettings = m_pMainCamera->GetAutoExposureSettings(),
			.autoExposure = m_pMainCamera->IsAutoExposureEnabled(),
		});
	UpdateData();
}
void pompeii::RenderSystem::BeginFrame()
{
	AddPendingObjects();
	FrustumCull();
}
void pompeii::RenderSystem::EndFrame()
{
	m_vVisibleModels.clear();
}

void pompeii::RenderSystem::SetRenderer(const std::shared_ptr<Renderer>& renderer)
{
	m_pRenderer = renderer;
}
pompeii::Renderer* pompeii::RenderSystem::GetRenderer() const
{
	return m_pRenderer.get();
}

//--------------------------------------------------
//    Helpers
//--------------------------------------------------
void pompeii::RenderSystem::FrustumCull()
{
	for (MeshRenderer* registeredModel : m_vRegisteredModels)
	{
		m_vVisibleModels.push_back(registeredModel);
	}
}
void pompeii::RenderSystem::AddPendingObjects()
{
	if (!m_vPendingModels.empty())
	{
		for (auto& pendingModel : m_vPendingModels)
			m_vRegisteredModels.emplace_back(pendingModel);
		m_vPendingModels.clear();
	}
}
void pompeii::RenderSystem::UpdateData()
{
	if (m_UpdateModels)
	{
		m_UpdateModels = false;

		std::vector<Image*> newTextures{};
		for (const auto& registeredModel : m_vRegisteredModels)
			if (registeredModel->pMeshFilter)
				for (auto& image : registeredModel->pMeshFilter->pMesh->images)
					newTextures.push_back(&image);
		m_pRenderer->UpdateTextures(newTextures);
	}
}
