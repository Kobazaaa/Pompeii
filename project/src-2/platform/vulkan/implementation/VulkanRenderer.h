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
		void InitializeVulkan();
		void CreateInstance();
		void CreateSurface();
		void CreateDevice(VkPhysicalDeviceFeatures2 features);
		void ChoosePhysicalDevice(VkPhysicalDeviceFeatures2 features);
		void CreateAllocator();
		void CreateFrameContext();
		void CreateSwapChain();
		void CreateCommandPool();

		void Submit();
	private:
		std::vector<std::unique_ptr<IRenderPass>> m_vRenderPasses{};

		IWindow* m_pWindow{ nullptr };

		VulkanContext m_Context{};
		VulkanCommandPool m_CommandPool{};
		std::vector<VulkanFrameContext> m_vFrames{};

		std::uint32_t m_CurrentFrame = 0;
	};
}

#endif // POMPEII_VULKAN_RENDERER_IMPLEMENTATION_H
