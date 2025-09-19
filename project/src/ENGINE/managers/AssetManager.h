#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

// -- Standard Library --
#include <unordered_map>
#include <memory>
#include <string>

// -- Pompeii Includes --
#include "Mesh.h" 
#include "Renderer.h"

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Asset Manager	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class AssetManager final
	{
	public:
		//--------------------------------------------------
		//    Renderer
		//--------------------------------------------------
		void SetRenderer(const std::shared_ptr<Renderer>& renderer);
		Renderer* GetRenderer() const;

		//--------------------------------------------------
		//    Assets
		//--------------------------------------------------
		Mesh* LoadMesh(const std::string& path);
		Mesh* GetMesh(const std::string& path);
		void UnloadMesh(const Mesh* pMesh);
		void UnloadAll();
		std::vector<Mesh*> GetAllMeshes() const;

	private:
		std::unordered_map<std::string, std::unique_ptr<Mesh>> m_vMeshRegistry{};
		std::shared_ptr<Renderer> m_pRenderer{};
	};
}

#endif // ASSET_MANAGER_H
