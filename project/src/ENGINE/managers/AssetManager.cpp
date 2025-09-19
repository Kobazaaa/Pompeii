// -- Pompeii Includes --
#include "AssetManager.h"

// -- Standard Library --
#include <ranges>

//--------------------------------------------------
//    Renderer
//--------------------------------------------------
void pompeii::AssetManager::SetRenderer(const std::shared_ptr<Renderer>& renderer)
{
	m_pRenderer = renderer;
}
pompeii::Renderer* pompeii::AssetManager::GetRenderer() const
{
	return m_pRenderer.get();
}

//--------------------------------------------------
//    Assets
//--------------------------------------------------
pompeii::Mesh* pompeii::AssetManager::LoadMesh(const std::string& path)
{
	if (!m_vMeshRegistry.contains(path))
	{
		m_vMeshRegistry[path] = std::make_unique<Mesh>(path);
		m_vMeshRegistry[path]->AllocateResources(m_pRenderer->GetContext());
	}
	return GetMesh(path);
}
pompeii::Mesh* pompeii::AssetManager::GetMesh(const std::string& path)
{
	if (m_vMeshRegistry.contains(path))
		return m_vMeshRegistry[path].get();
	return nullptr;
}

void pompeii::AssetManager::UnloadMesh(const Mesh* pMesh)
{
	auto it = std::ranges::find_if(
			m_vMeshRegistry,
			[pMesh](const std::pair<const std::string, std::unique_ptr<Mesh>>& pair)
			{
				return pMesh == pair.second.get();
			});
	if (it != m_vMeshRegistry.end())
	{
		m_pRenderer->GetContext().device.WaitIdle();
		it->second->Destroy(m_pRenderer->GetContext());
		m_vMeshRegistry.erase(it);
	}
}
void pompeii::AssetManager::UnloadAll()
{
	m_pRenderer->GetContext().device.WaitIdle();
	for (auto& mesh : m_vMeshRegistry | std::views::values)
		mesh->Destroy(m_pRenderer->GetContext());
	m_vMeshRegistry.clear();
}
std::vector<pompeii::Mesh*> pompeii::AssetManager::GetAllMeshes() const
{
	std::vector<Mesh*> res(m_vMeshRegistry.size());
	for (const auto& val : m_vMeshRegistry | std::views::values)
		res.emplace_back(val.get());
	return res;
}
