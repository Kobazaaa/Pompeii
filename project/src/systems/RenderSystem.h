#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

// -- Standard Library --
#include <vector>
#include <memory>

// -- Pompeii Includes --
#include "ISystem.h"
#include "Renderer.h"

// -- Forward Declarations --
namespace pompeii
{
	class ModelRenderer;
	class Camera;
	class LightComponent;
	struct GPULight;
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
		void RegisterModel(ModelRenderer& model, const std::string& path);
		void UnregisterModel(const ModelRenderer& model);

		//--------------------------------------------------
		//    Camera
		//--------------------------------------------------
		Camera* GetMainCamera() const;
		void SetMainCamera(Camera& camera);

		//--------------------------------------------------
		//    Lights
		//--------------------------------------------------
		void RegisterLight(LightComponent& light);
		void UnregisterLight(const LightComponent& light);

		//--------------------------------------------------
		//    Interface
		//--------------------------------------------------
		void SetRenderer(std::unique_ptr<Renderer>&& renderer) { m_pRenderer = std::move(renderer); }
		Renderer* GetRenderer() const;
		void BeginFrame() override;
		void Update() override;
		void Render();
		void EndFrame() override;

	private:
		std::vector<ModelRenderer*> m_vRegisteredModels{};
		std::vector<ModelRenderer*> m_vVisibleModels{};
		std::vector<LightComponent*> m_vRegisteredLights{};
		Camera* m_pMainCamera{};
		std::unique_ptr<Renderer> m_pRenderer{};

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void FrustumCull();
	};
}


#endif // RENDER_SYSTEM_H