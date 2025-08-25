// -- Pompeii Includes --
#include "MeshFilter.h"
#include "AssetManager.h"
#include "ServiceLocator.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  MeshFilter	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::MeshFilter::MeshFilter(SceneObject& sceneObj)
	: Component(sceneObj, "MeshFilter")
{}
pompeii::MeshFilter::~MeshFilter()
{
	ServiceLocator::Get<AssetManager>().UnloadMesh(pMesh);
}


//--------------------------------------------------
//    Loop
//--------------------------------------------------
void pompeii::MeshFilter::Start()
{
	Component::Start();
}
void pompeii::MeshFilter::OnInspectorDraw()
{
	Component::OnInspectorDraw();
}
