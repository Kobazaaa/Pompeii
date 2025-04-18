#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

// -- Vulkan Includes --
#include "vulkan/vulkan.h"

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "Image.h"

// -- Forward Declarations --
namespace pom
{
	class CommandPool;
	class Window;
	class SwapChain;
	struct Context;
}

namespace pom
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
		void Build(Context& context, const Window& window, SwapChain& swapChain, CommandPool& cmdPool);

	private:
		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window& window);
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
		SwapChain() = default;
		void Destroy(const Context& context) const;
		void Recreate(Context& context, const Window& window, CommandPool& cmdPool);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkSwapchainKHR& GetHandle() const;
		std::vector<VkImage>& GetImageHandles();
		std::vector<VkImageView>& GetViewHandles();
		uint32_t GetImageCount() const;
		const Image& GetDepthImage() const;

		VkFormat GetFormat() const;
		VkExtent2D GetExtent() const;

	private:
		VkSwapchainKHR				m_SwapChain{ VK_NULL_HANDLE };
		Image						m_DepthImage{};

		std::vector<VkImage>		m_vSwapChainImages{};
		std::vector<VkImageView>	m_vSwapChainImageViews{};
		VkFormat					m_SwapChainImageFormat{};
		VkExtent2D					m_SwapChainExtent{};

		friend class SwapChainBuilder;
		SwapChainBuilder			m_OriginalBuilder{};
	};
}
#endif // SWAPCHAIN_H