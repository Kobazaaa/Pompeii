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
	: Component(sceneObj)
{
	m_pModel->LoadModel(path);
	ServiceLocator::Get<RenderSystem>().RegisterModel(*this);
}
pompeii::ModelRenderer::~ModelRenderer()
{
	m_pModel->Destroy(ServiceLocator::Get<Renderer>().GetContext());
	ServiceLocator::Get<RenderSystem>().UnregisterModel(*this);
}


//--------------------------------------------------
//    Loop
//--------------------------------------------------
void pompeii::ModelRenderer::Start()
{
	const auto& ctx = ServiceLocator::Get<Renderer>().GetContext();
	m_pModel->AllocateResources(ctx, true);
	ServiceLocator::Get<Renderer>().UpdateTextures();
}
void pompeii::ModelRenderer::OnImGuiRender()
{
}

//--------------------------------------------------
//    Accessors
//--------------------------------------------------
pompeii::Model* pompeii::ModelRenderer::GetModel() const
{
	return m_pModel.get();
}
