// -- Pompeii Includes --
#include "MeshRenderer.h"
#include "ServiceLocator.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Model	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::MeshRenderer::MeshRenderer(SceneObject& sceneObj, MeshFilter& filter)
	: Component(sceneObj, "MeshRenderer")
{
	pMeshFilter = &filter;
	ServiceLocator::Get<RenderSystem>().RegisterMeshRenderer(*this);
	GetSceneObject().GetScene().GrowAABB(pMeshFilter->pMesh->aabb);
}
pompeii::MeshRenderer::~MeshRenderer()
{
	ServiceLocator::Get<RenderSystem>().UnregisterMeshRenderer(*this);
}


//--------------------------------------------------
//    Loop
//--------------------------------------------------
void pompeii::MeshRenderer::Start()
{
}
void pompeii::MeshRenderer::OnInspectorDraw()
{
}
