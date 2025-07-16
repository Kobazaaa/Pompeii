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

