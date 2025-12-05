#ifndef RENDERER_H
#define RENDERER_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "Context.h"
#include "SwapChain.h"
#include "SyncManager.h"

#include "ShadowPass.h"
#include "DepthPrePass.h"
#include "GeometryPass.h"
#include "LightingPass.h"
#include "BlitPass.h"

#include "EnvironmentMap.h"
#include "Light.h"
#include "Mesh.h"
#include "RenderingItems.h"
#include "GPUCamera.h"

// -- Forward Declarations --
namespace pompeii
{
	struct GPULight;
	class IWindow;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Renderer	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Renderer final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit Renderer();
		~Renderer();
		Renderer(const Renderer& other) = delete;
		Renderer(Renderer&& other) noexcept = delete;
		Renderer& operator=(const Renderer& other) = delete;
		Renderer& operator=(Renderer&& other) noexcept = delete;

		void Initialize(IWindow* pWindow);
		void Deinitialize();

		//--------------------------------------------------
		//    Loop
		//--------------------------------------------------
		bool StartFrame();
		void RecordFrame();
		void SubmitFrame();
		void EndFrame();

		void ClearQueue();
		void SubmitRenderItem(const RenderItem& item);
		void SubmitLightItem(const LightItem& item);

		void SetCamera(const CameraData& camera);

		//--------------------------------------------------
		//    Accessors
		//--------------------------------------------------
		Context& GetContext();
		Image& GetCurrentSwapChainImage();
		Image& GetCurrentOutputImage();
		std::vector<Image>& GetOutputImages();

		void UpdateLights(const std::vector<Light*>& lights);
		void UpdateTextures(const std::vector<Image*>& textures);
		void UpdateEnvironmentMap() const;

	private:
		//--------------------------------------------------
		//    Vulkan Specific
		//--------------------------------------------------
		void InitializeVulkan();

		// -- Vulkan Context --
		Context m_Context { };
		std::vector<RenderItem> m_vRenderItems;
		std::vector<LightItem> m_vLightItems;
		uint32_t padding[2]{};
		CameraData m_Camera{};


		// -- SwapChain --
		SwapChain					m_SwapChain				{ };
		std::vector<Image>			m_vDepthImages			{ };
		std::vector<Image>			m_vRenderTargets		{ };
		std::vector<Image>			m_vOutputImages			{ };

		// -- Sync --
		SyncManager					m_SyncManager			{ };

		// -- Passes --
		ShadowPass					m_ShadowPass			{ };
		DepthPrePass				m_DepthPrePass			{ };
		GeometryPass				m_GeometryPass			{ };
		LightingPass				m_LightingPass			{ };
		BlitPass					m_BlitPass				{ };

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void RecreateSwapChain();
		void CreateDepthResources(const Context& context, VkExtent2D extent);
		void CreateRenderTargetResources(const Context& context, VkExtent2D extent);
		void CreateOutputResources(const Context& context, VkExtent2D extent);

		// -- Other --
		IWindow*			m_pWindow			{ };
		EnvironmentMap		m_EnvMap			{ };
	};
}

#endif // RENDERER_H