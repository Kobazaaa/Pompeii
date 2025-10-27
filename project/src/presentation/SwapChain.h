#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

// -- Vulkan Includes --
#include "vulkan/vulkan.h"

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "Image.h"

// -- Forward Declarations --
namespace pompeii
{
	class CommandPool;
	class SwapChain;
	struct Context;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  SwapChainBuilder	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class SwapChainBuilder final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		SwapChainBuilder() = default;


		//--------------------------------------------------
		//    Builder
		//--------------------------------------------------
		SwapChainBuilder& SetDesiredImageCount(uint32_t count);
		SwapChainBuilder& SetImageUsage(VkImageUsageFlags usage);
		SwapChainBuilder& SetImageArrayLayers(uint32_t layerCount);
		void Build(Context& context, VkSurfaceKHR surface, VkExtent2D windowExtent, SwapChain& swapChain);

	private:
		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D windowExtent);
		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		uint32_t m_DesiredImageCount{};
		VkSwapchainCreateInfoKHR m_CreateInfo{};
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  SwapChain
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class SwapChain final
	{
	public:

		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit SwapChain() = default;
		~SwapChain() = default;
		SwapChain(const SwapChain& other) = delete;
		SwapChain(SwapChain&& other) noexcept = delete;
		SwapChain& operator=(const SwapChain& other) = delete;
		SwapChain& operator=(SwapChain&& other) noexcept = delete;

		void Destroy(const Context& context);
		void Recreate(Context& context, VkSurfaceKHR surface, VkExtent2D windowExtent);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkSwapchainKHR& GetHandle() const;
		std::vector<Image>& GetImages();
		uint32_t GetImageCount() const;

		VkFormat GetFormat() const;
		VkExtent2D GetExtent() const;

	private:
		VkSwapchainKHR				m_SwapChain{ VK_NULL_HANDLE };

		std::vector<Image>			m_vSwapChainImages{};
		VkFormat					m_SwapChainImageFormat{};
		VkExtent2D					m_SwapChainExtent{};

		friend class SwapChainBuilder;
		SwapChainBuilder			m_OriginalBuilder{};
	};
}
#endif // SWAPCHAIN_H