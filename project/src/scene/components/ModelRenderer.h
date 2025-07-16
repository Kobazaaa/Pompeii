#ifndef MODEL_MESH_H
#define MODEL_MESH_H

// -- Pompeii Includes --
#include "Model.h"
#include "Component.h"

// -- Forward Declarations --
struct aiNode;
struct aiScene;
struct aiMesh;
namespace pompeii
{
	class CommandBuffer;
	class PipelineLayout;
	struct Context;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Model	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class ModelRenderer final : public Component
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit ModelRenderer(SceneObject& sceneObj, const std::string& path);
		~ModelRenderer() override;

		ModelRenderer(const ModelRenderer& other) = delete;
		ModelRenderer(ModelRenderer&& other) noexcept = delete;
		ModelRenderer& operator=(const ModelRenderer& other) = delete;
		ModelRenderer& operator=(ModelRenderer&& other) noexcept = delete;

		//--------------------------------------------------
		//    Loop
		//--------------------------------------------------
		void Start() override;
		void OnImGuiRender() override;

		//--------------------------------------------------
		//    Accessors
		//--------------------------------------------------
		Model* GetModel() const;

	private:
		std::unique_ptr<Model> m_pModel{ std::make_unique<Model>() };
	};
}

#endif // MODEL_MESH_H