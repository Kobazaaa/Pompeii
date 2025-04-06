#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H
#include <vector>

#include "CommandPool.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "Window.h"
#include "Image.h"

namespace pom
{
	class SwapChain;

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
		void Build(Device& device, const VmaAllocator& allocator, PhysicalDevice& physicalDevice, const Window& window, SwapChain& swapChain, CommandPool& cmdPool);

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
		void Destroy(Device& device, const VmaAllocator& allocator) const;
		void Recreate(Device& device, const VmaAllocator& allocator, PhysicalDevice& physicalDevice, const Window& window, CommandPool& cmdPool);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		VkSwapchainKHR& GetSwapChain();
		std::vector<Image>& GetImages();
		uint32_t GetImageCount() const;

		VkFormat GetFormat() const;
		VkExtent2D GetExtent() const;

		Image& GetDepthImage();

	private:
		VkSwapchainKHR				m_SwapChain{ VK_NULL_HANDLE };
		Image						m_DepthImage{};

		std::vector<Image>			m_vSwapChainImages{};
		VkFormat					m_SwapChainImageFormat{};
		VkExtent2D					m_SwapChainExtent{};

		friend class SwapChainBuilder;
		SwapChainBuilder			m_OriginalBuilder{};
	};
}
#endif // SWAPCHAIN_H