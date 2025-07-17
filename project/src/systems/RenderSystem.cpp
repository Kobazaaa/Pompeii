// -- Pompeii Includes --
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
//    Lights
//--------------------------------------------------
void pompeii::RenderSystem::RegisterLight(LightComponent& light)
{
	if (std::ranges::find(m_vRegisteredLights, &light) != m_vRegisteredLights.end())
		return;
	m_vRegisteredLights.emplace_back(&light);
	auto handle = m_pRenderer->CreateLight(light.lightData);
	light.m_LightHandle = handle;
	m_pRenderer->UpdateLights();

}
void pompeii::RenderSystem::UnregisterLight(const LightComponent& light)
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
