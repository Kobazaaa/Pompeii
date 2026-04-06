#ifndef POMPEII_VULKAN_RENDERER_IMPLEMENTATION_H
#define POMPEII_VULKAN_RENDERER_IMPLEMENTATION_H

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "VulkanFrameContext.h"
#include "IRenderPass.h"
#include "IRenderer.h"
#include "VulkanCommandPool.h"
#include "VulkanContext.h"
#include "VulkanDebugMessenger.h"
#include "VulkanPipelineCache.h"
#include "VulkanResourceManager.h"

// -- Forward Declarations --
namespace pompeii
{
	class IWindow;
}

namespace pompeii::vulkan
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    VulkanRenderer
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class VulkanRenderer final : public IRenderer
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit VulkanRenderer(IWindow* pWindow);
		~VulkanRenderer() override;

		VulkanRenderer(const VulkanRenderer& other) = delete;
		VulkanRenderer(VulkanRenderer&& other) noexcept = delete;
		VulkanRenderer& operator=(const VulkanRenderer& other) = delete;
		VulkanRenderer& operator=(VulkanRenderer&& other) noexcept = delete;


		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		void AddRenderPass(std::unique_ptr<IRenderPass> pass) override;

		void BeginFrame() override;
		void Execute() override;
		void EndFrame() override;

		void Shutdown() override;

	private:
		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void InitializeHelpers();
		void InitializeVulkan();
		void CreateInstance();
		void CreateSurface();
		void CreateDevice(VkPhysicalDeviceFeatures2 features);
		void ChoosePhysicalDevice(VkPhysicalDeviceFeatures2 features);
		void CreateAllocator();
		void CreateFrameContext();
		void CreateSwapChain();
		void CreateCommandPool();

	private:
		VulkanContext m_Context{};
		std::unique_ptr<VulkanDebugMessenger> m_pDebugMessenger { nullptr };

		VulkanCommandPool m_CommandPool{};
		std::vector<VulkanFrameContext> m_vFrames{};

		VulkanResourceManager m_ResourceManager{};
		VulkanPipelineCache m_PipelineCache{};

		std::vector<std::unique_ptr<IRenderPass>> m_vRenderPasses{};

		IWindow* m_pWindow{ nullptr };
		std::uint32_t m_CurrentFrame = 0;
	};
}

#endif // POMPEII_VULKAN_RENDERER_IMPLEMENTATION_H
