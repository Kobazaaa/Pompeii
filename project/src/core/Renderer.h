#ifndef RENDERER_H
#define RENDERER_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "BlitPass.h"
#include "Context.h"
#include "DepthPrePass.h"
#include "SwapChain.h"
#include "SyncManager.h"
#include "UIPass.h"
#include "GeometryPass.h"
#include "LightingPass.h"

// -- Forward Declarations --
namespace pompeii
{
	class Window;
	class Camera;
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
		explicit Renderer(Camera* pCamera, Window* pWindow);
		~Renderer();
		Renderer(const Renderer& other) = delete;
		Renderer(Renderer&& other) noexcept = delete;
		Renderer& operator=(const Renderer& other) = delete;
		Renderer& operator=(Renderer&& other) noexcept = delete;

		//--------------------------------------------------
		//    Loop
		//--------------------------------------------------
		void Update();
		void Render();

	private:
		//--------------------------------------------------
		//    Vulkan Specific
		//--------------------------------------------------
		void InitializeVulkan();

		// -- Vulkan Context --
		Context						m_Context				{ };

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
		Camera*				m_pCamera			{ };
		Scene*				m_pScene			{ };

		// -- Frame Counter --
		uint32_t			m_CurrentFrame		{ 0 };
	};
}

#endif // RENDERER_H