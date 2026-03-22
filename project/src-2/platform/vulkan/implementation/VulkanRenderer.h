#ifndef POMPEII_VULKAN_RENDERER_IMPLEMENTATION_H
#define POMPEII_VULKAN_RENDERER_IMPLEMENTATION_H

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "IRenderPass.h"
#include "IRenderer.h"
#include "VulkanCommandPool.h"
#include "VulkanContext.h"

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
		explicit VulkanRenderer();
		~VulkanRenderer() override = default;

		VulkanRenderer(const VulkanRenderer& other) = default;
		VulkanRenderer(VulkanRenderer&& other) noexcept = default;
		VulkanRenderer& operator=(const VulkanRenderer& other) = default;
		VulkanRenderer& operator=(VulkanRenderer&& other) noexcept = default;


		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		void AddRenderPass(std::unique_ptr<IRenderPass> pass) override;

		void BeginFrame() override;
		void Execute() override;
		void EndFrame() override;

	private:
		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void InitializeVulkan();
		void CreateInstance();
		void CreateDevice(VkPhysicalDeviceFeatures2 features);
		void ChoosePhysicalDevice(VkPhysicalDeviceFeatures2 features);
		void CreateAllocator();
		void CreateSwapChain();
		void CreateCommandPool();

		VulkanContext m_Context{};
		VulkanCommandPool m_CommandPool{};
		std::vector<std::unique_ptr<ICommandBuffer>> m_vCommandBuffers{};

		std::uint32_t m_CurrentFrame = 0;

		std::vector<std::unique_ptr<IRenderPass>> m_vRenderPasses{};
	};
}

#endif // POMPEII_VULKAN_RENDERER_IMPLEMENTATION_H
