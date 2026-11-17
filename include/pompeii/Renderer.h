#ifndef RENDERER_H
#define RENDERER_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <span>
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
#include "Handle.h"
#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "ResourcePool.h"
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

		void ExecuteBeforeCommandBuffer(const std::function<void()>& func);
		void ExecuteAfterCommandBuffer(const std::function<void()>& func);

		//--------------------------------------------------
		//    Resources
		//--------------------------------------------------
		// -- Mesh --
		MeshHandle CreateMesh(const MeshCreateInfo& createInfo);
		void UpdateMeshVertices(MeshHandle handle, std::span<const Vertex> vertices);
		void UpdateMeshIndices(MeshHandle handle, std::span<const uint32_t> indices);
		void DestroyMesh(MeshHandle handle);

		// -- Texture --
		TextureHandle CreateTexture(const TextureCreateInfo& createInfo);
		void DestroyTexture(TextureHandle handle);

		// -- Material --
		MaterialHandle CreateMaterial(const MaterialCreateInfo& createInfo);
		void UpdateMaterial(MaterialHandle handle, const MaterialCreateInfo& createInfo);
		void DestroyMaterial(MaterialHandle handle);

		// -- Buffer --
		BufferHandle CreateBuffer(const BufferCreateInfo& createInfo);
		void UpdateBufferData(BufferHandle handle, const void* data, uint32_t size, uint32_t dstOffset = 0);
		void DestroyBuffer(BufferHandle handle);

		// -- Light --
		LightHandle CreateLight(const LightCreateInfo& createInfo);
		void UpdateLight(LightHandle handle, const LightCreateInfo& createInfo);
		void DestroyLight(LightHandle handle);

		// -- Access --
		[[nodiscard]] bool IsValid(MeshHandle handle) const;
		[[nodiscard]] bool IsValid(TextureHandle handle) const;
		[[nodiscard]] bool IsValid(MaterialHandle handle) const;
		[[nodiscard]] bool IsValid(BufferHandle handle) const;
		[[nodiscard]] bool IsValid(LightHandle handle) const;
		[[nodiscard]] const Mesh* TryGetMesh(MeshHandle handle) const;
		[[nodiscard]] const Buffer* TryGetBuffer(BufferHandle handle) const;

		//--------------------------------------------------
		//    Mutators
		//--------------------------------------------------
		void ResizeOutput(uint32_t w, uint32_t h);
		void UpdateEnvironmentMap() const;
		void SetCamera(const CameraData& camera);

		//--------------------------------------------------
		//    Accessors
		//--------------------------------------------------
		Context& GetContext();
		Image& GetCurrentSwapChainImage();
		Image& GetCurrentOutputImage();
		std::vector<Image>& GetOutputImages();

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

		// -- Resource Pools --
		ResourcePool<Mesh>		m_MeshPool		{ };
		ResourcePool<Image>		m_TexturePool	{ };
		ResourcePool<Material>	m_MaterialPool	{ };
		ResourcePool<Buffer>	m_BufferPool	{ };
		ResourcePool<Light>		m_LightPool		{ };
		bool					m_LightsDirty	{ false };

		// -- SwapChain --
		SwapChain					m_SwapChain				{ };
		std::vector<Image>			m_vDepthImages			{ };
		std::vector<Image>			m_vRenderTargets		{ };
		std::vector<Image>			m_vOutputImages			{ };

		// -- Sync --
		SyncManager					m_SyncManager			{ };

		using FuncVector = std::vector<std::function<void()>>;
		FuncVector m_BeforeCommandBufferExecutions			{ };
		FuncVector m_AfterCommandBufferExecutions			{ };

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
		void RefreshLightsIfDirty();

		// -- Other --
		IWindow*			m_pWindow			{ };
		EnvironmentMap		m_EnvMap			{ };
	};
}

#endif // RENDERER_H