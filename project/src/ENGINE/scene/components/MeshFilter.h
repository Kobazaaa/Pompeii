#ifndef MESH_FILTER_H
#define MESH_FILTER_H

// -- Pompeii Includes --
#include "Mesh.h"
#include "Component.h"

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  MeshFilter	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class MeshFilter final : public Component
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit MeshFilter(SceneObject& sceneObj);
		~MeshFilter() override;

		MeshFilter(const MeshFilter& other) = delete;
		MeshFilter(MeshFilter&& other) noexcept = delete;
		MeshFilter& operator=(const MeshFilter& other) = delete;
		MeshFilter& operator=(MeshFilter&& other) noexcept = delete;

		//--------------------------------------------------
		//    Loop
		//--------------------------------------------------
		void Start() override;
		void OnInspectorDraw() override;

		//--------------------------------------------------
		//    Data
		//--------------------------------------------------
		Mesh* pMesh{};
	};
}

#endif // MESH_FILTER_H