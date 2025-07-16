#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "ISystem.h"

// -- Forward Declarations --
namespace pompeii
{
	class ModelRenderer;
	class Camera;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Render System	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class RenderSystem final : public ISystem
	{
	public:
		//--------------------------------------------------
		//    Models
		//--------------------------------------------------
		void RegisterModel(ModelRenderer& model);
		void UnregisterModel(const ModelRenderer& model);
		const std::vector<ModelRenderer*>& GetVisibleModels() const;
		const std::vector<ModelRenderer*>& GetAllModels() const;

		//--------------------------------------------------
		//    Textures
		//--------------------------------------------------
		uint32_t GetTextureCount() const;

		//--------------------------------------------------
		//    Camera
		//--------------------------------------------------
		Camera* GetMainCamera() const;
		void SetMainCamera(Camera& camera);

		//--------------------------------------------------
		//    Interface
		//--------------------------------------------------
		void BeginFrame() override;
		void Update() override;
		void EndFrame() override;

	private:
		std::vector<ModelRenderer*> m_vRegisteredModels{};
		std::vector<ModelRenderer*> m_vVisibleModels{};
		Camera* m_pMainCamera{};

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void FrustumCull();
	};
}


#endif // RENDER_SYSTEM_H