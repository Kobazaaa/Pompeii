// -- Pompeii Includes --
#include "Scene.h"

// -- Standard Library --
#include <numeric>
#include <ranges>

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Base Scene	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::Scene::Scene(std::string sceneName)
	: name(std::move(sceneName))
{}


//--------------------------------------------------
//    Adding & Removing SceneObjects
//--------------------------------------------------
pompeii::SceneObject& pompeii::Scene::AddEmpty(const std::string& sceneName)
{
	m_vPendingObjects.emplace_back(std::make_unique<SceneObject>(*this, sceneName));
	return *m_vPendingObjects.back();
}

//--------------------------------------------------
//    Loop
//--------------------------------------------------
void pompeii::Scene::Start() const
{
	for (auto& object : m_vObjects)
	{
		object->Start();
	}
}
void pompeii::Scene::Update()
{
	for (auto& object : m_vObjects)
	{
		if (!object->IsActive())
			continue;
		object->Update();
	}
	AddPendingObjects();
	CleanupDeletedObjects();
}
void pompeii::Scene::OnImGuiRender() const
{
	for (const auto& object : m_vObjects)
	{
		if (!object->IsActive())
			continue;
		object->OnImGuiRender();
	}
}


//--------------------------------------------------
//    Registrators
//--------------------------------------------------
// -- Model --
void pompeii::Scene::RegisterModel(Model& model)
{
	if (std::ranges::find(m_vModelComponents, &model) != m_vModelComponents.end())
		return;
	m_vModelComponents.emplace_back(&model);
	m_AABB.GrowToInclude(model.aabb);
}
const std::vector<pompeii::Model*>& pompeii::Scene::GetModels() const
{
	return m_vModelComponents;
}
uint32_t pompeii::Scene::GetImageCount() const
{
	return std::accumulate(m_vModelComponents.begin(), m_vModelComponents.end(), 0u, [](uint32_t res, const Model* m2)
		{
			return res + static_cast<uint32_t>(m2->images.size());
		});
}

// -- Lights --
void pompeii::Scene::RegisterLight(Light& light)
{
	if (std::ranges::find(m_vLightComponents, &light) != m_vLightComponents.end())
		return;
	m_vLightComponents.emplace_back(&light);
}
std::vector<pompeii::Light*>& pompeii::Scene::GetLights()
{
	return m_vLightComponents;
}
std::vector<pompeii::GPULight> pompeii::Scene::GetLightsGPU() const
{
	std::vector<pompeii::GPULight> res{};
	res.reserve(m_vLightComponents.size());

	uint32_t matrixIdx = 0;
	uint32_t directionalCounter = 0;
	uint32_t pointCounter = 0;
	for (auto& l : m_vLightComponents)
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
uint32_t pompeii::Scene::GetLightsCount() const
{
	return static_cast<uint32_t>(m_vLightComponents.size());
}
std::vector<glm::mat4> pompeii::Scene::GetLightMatrices() const
{
	std::vector<glm::mat4> res{};
	for (auto& l : m_vLightComponents)
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
//    Accessors
//--------------------------------------------------
std::vector<pompeii::SceneObject*> pompeii::Scene::GetObjectsByName(const std::string& objectName) const
{
	//todo Preferably get rid of string comparisons
	std::vector<SceneObject*> result{};
	for (auto& object : m_vObjects)
	{
		if (object->name == objectName)
			result.push_back(object.get());
	}
	for (auto& object : m_vPendingObjects)
	{
		if (object->name == objectName)
			result.push_back(object.get());
	}
	return result;
}
const pompeii::AABB& pompeii::Scene::GetAABB() const
{
	return m_AABB;
}

void pompeii::Scene::CleanupDeletedObjects()
{
	std::erase_if(m_vObjects,
		[](const std::unique_ptr<SceneObject>& object)
		{
			return object->IsFlaggedForDestruction();
		});
}
void pompeii::Scene::AddPendingObjects()
{
	// separate for loops to ensure that all objects in m_vPendingObjects are in a valid state, since it's possible to query
	// m_vPendingObjects in the start function of SceneObjects (e.g. asking for all SO with tag or name).
	for (const auto& object : m_vPendingObjects)
		object->Start();
	for (auto& object : m_vPendingObjects)
		m_vObjects.push_back(std::move(object));
	m_vPendingObjects.clear();
}

