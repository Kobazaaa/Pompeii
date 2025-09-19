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
    if (pMeshFilter)
        ImGui::Text("Mesh Filter: %s", pMeshFilter->GetSceneObject().name.c_str());
    else
        ImGui::Text("Mesh Filter: None");

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MeshFilter"))
        {
            Component* dropped = *static_cast<Component**>(payload->Data);
            if (MeshFilter* mf = dynamic_cast<MeshFilter*>(dropped))
                if (mf != pMeshFilter)
                    pMeshFilter = mf;
        }
        ImGui::EndDragDropTarget();
    }
}
