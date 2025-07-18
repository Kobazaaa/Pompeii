// -- Pompeii Includes --
#include "Transform.h"
#include "RenderSystem.h"
#include "ModelRenderer.h"
#include "LightComponent.h"

// -- Standard Library --
#include <numeric>

//--------------------------------------------------
//    Models
//--------------------------------------------------
void pompeii::RenderSystem::RegisterModel(ModelRenderer& model, const std::string& path)
{
	if (std::ranges::find(m_vRegisteredModels, &model) != m_vRegisteredModels.end())
		return;
	ModelCPU modelData{path};
	m_vRegisteredModels.emplace_back(&model);
	auto handle = m_pRenderer->CreateModel(modelData);
	model.m_ModelHandle = handle;
	m_pRenderer->UpdateTextures();
}
void pompeii::RenderSystem::UnregisterModel(const ModelRenderer& model)
{
	std::erase_if(m_vRegisteredModels, [&](const ModelRenderer* pModel)
	{
		if (pModel == &model)
		{
			m_pRenderer->DestroyModel(model.GetModelHandle());
			return true;
		}
		return false;
	});
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
	for (auto& m : m_vVisibleModels)
	{
		m_pRenderer->AddRenderInstance(m->GetModelHandle(), m->GetTransform().GetMatrix());
	}
}
void pompeii::RenderSystem::Render()
{
	m_pRenderer->Render();
}
void pompeii::RenderSystem::BeginFrame()
{
	FrustumCull();
}
void pompeii::RenderSystem::EndFrame()
{
	m_vVisibleModels.clear();
	m_pRenderer->ClearRenderInstances();
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
	for (ModelRenderer* pModel : m_vRegisteredModels)
	{
		m_vVisibleModels.push_back(pModel);
	}
}
