#ifndef RENDERER_H
#define RENDERER_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <vector>
#include <memory>

// -- Pompeii Includes --
#include "Context.h"
#include "SwapChain.h"
#include "SyncManager.h"

#include "DepthPrePass.h"
#include "GeometryPass.h"
#include "LightingPass.h"
#include "BlitPass.h"
#include "UIPass.h"

#include "EnvironmentMap.h"
#include "Light.h"
#include "Model.h"

// -- Forward Declarations --
namespace pompeii
{
	struct GPULight;
	class Window;
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
		explicit Renderer(Window* pWindow);
		~Renderer();
		Renderer(const Renderer& other) = delete;
		Renderer(Renderer&& other) noexcept = delete;
		Renderer& operator=(const Renderer& other) = delete;
		Renderer& operator=(Renderer&& other) noexcept = delete;

		//--------------------------------------------------
		//    Loop
		//--------------------------------------------------
		void Render();
		void ClearRenderInstances();
		void AddRenderInstance(ModelHandle handle, const glm::mat4& transform);

		ModelHandle CreateModel(const ModelCPU& modelCPU);
		void DestroyModel(ModelHandle handle);

		LightHandle CreateLight(const LightCPU& lightData);
		void DestroyLight(LightHandle handle);

		void InsertUI(const std::function<void()>& func);

		//--------------------------------------------------
		//    Accessors
		//--------------------------------------------------
		Context& GetContext();
		void UpdateLights();
		void UpdateTextures();
		void UpdateEnvironmentMap() const;

	private:
		//--------------------------------------------------
		//    Vulkan Specific
		//--------------------------------------------------
		void InitializeVulkan();

		// -- Vulkan Context --
		Context m_Context				{ };
		std::unordered_map<ModelHandle, std::unique_ptr<ModelGPU>> m_vModelRegistry;
		std::unordered_map<LightHandle, std::unique_ptr<LightGPU>> m_vLightRegistry;
		std::vector<RenderInstance> m_vRenderInstances;

		// -- SwapChain --
		SwapChain					m_SwapChain				{ };
		std::vector<Image>			m_vDepthImages			{ };
		std::vector<Image>			m_vRenderTargets		{ };
		uint32_t					m_MaxFramesInFlight		{ 3 };

		// -- Sync --
		SyncManager					m_SyncManager			{ };

		// -- Passes --
		DepthPrePass				m_DepthPrePass			{ };
		GeometryPass				m_GeometryPass			{ };
		LightingPass				m_LightingPass			{ };
		BlitPass					m_BlitPass				{ };
		UIPass						m_UIPass				{ };

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void RecreateSwapChain();
		void CreateDepthResources(const Context& context, VkExtent2D extent);
		void CreateRenderTargetResources(const Context& context, VkExtent2D extent);
		void RecordCommandBuffer(CommandBuffer& commandBuffer, uint32_t imageIndex);

		// -- Other --
		Window*				m_pWindow			{ };
		EnvironmentMap		m_EnvMap			{ };

		// -- Frame Counter --
		uint32_t			m_CurrentFrame		{ 0 };
	};
}

#endif // RENDERER_H