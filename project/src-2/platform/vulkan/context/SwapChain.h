#ifndef POMPEII_VULKAN_SWAP_CHAIN_H
#define POMPEII_VULKAN_SWAP_CHAIN_H

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "VulkanImage.h"

// -- Forward Declarations --
namespace pompeii::vulkan
{
	class VulkanFrameContext;
	struct VulkanContext;
}

namespace pompeii::vulkan
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    SwapChainSupportDetails
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct SwapChainDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    VulkanSwapChain
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class VulkanSwapChain final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit VulkanSwapChain() = default;
		explicit VulkanSwapChain(VulkanContext& context, uint32_t width, uint32_t height, uint32_t buffers);
		~VulkanSwapChain();

		VulkanSwapChain(const VulkanSwapChain& other) = default;
		VulkanSwapChain(VulkanSwapChain&& other) noexcept = default;
		VulkanSwapChain& operator=(const VulkanSwapChain& other) = default;
		VulkanSwapChain& operator=(VulkanSwapChain&& other) noexcept = default;

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		bool AcquireNextImage(const VulkanFrameContext& frame);
		IImage* GetCurrentBackBuffer();
		bool Present(const VulkanFrameContext& frame);
		void Resize(uint32_t width, uint32_t height);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		uint32_t GetImageCount() const;
		uint32_t GetCurrentImageIndex() const;

	private:
		void CreateSwapChain(uint32_t width, uint32_t height, uint32_t buffers);
		void CreateImageViews();
		void CreateBackBuffers();
		void CleanUp();

		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D windowExtent);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);


		VkSwapchainKHR m_SwapChain				{ VK_NULL_HANDLE };

		std::vector<VkImage> m_vImages			{};
		std::vector<VkImageView> m_vImageViews	{};
		std::vector<VulkanImage> m_vBackBuffers	{};

		VkFormat m_Format;
		VkExtent2D m_Extent;

		uint32_t m_CurrentImageIndex			{0};

		VulkanContext* m_pContext				{ nullptr };
	};
}

#endif // POMPEII_VULKAN_SWAP_CHAIN_H
