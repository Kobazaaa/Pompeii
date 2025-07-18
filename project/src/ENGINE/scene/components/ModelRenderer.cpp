// -- Pompeii Includes --
#include "ModelRenderer.h"
#include "ServiceLocator.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Model	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::ModelRenderer::ModelRenderer(SceneObject& sceneObj, const std::string& path)
	: Component(sceneObj, "ModelRenderer")
{
	ServiceLocator::Get<RenderSystem>().RegisterModel(*this, path);
}
pompeii::ModelRenderer::~ModelRenderer()
{
	ServiceLocator::Get<RenderSystem>().UnregisterModel(*this);
}


//--------------------------------------------------
//    Loop
//--------------------------------------------------
void pompeii::ModelRenderer::Start()
{
}
void pompeii::ModelRenderer::OnInspectorDraw()
{
}


//--------------------------------------------------
//    Accessors
//--------------------------------------------------
ModelHandle pompeii::ModelRenderer::GetModelHandle() const
{
	return m_ModelHandle;
}
