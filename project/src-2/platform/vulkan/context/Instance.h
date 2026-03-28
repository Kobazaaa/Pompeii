#ifndef POMPEII_VULKAN_INSTANCE_H
#define POMPEII_VULKAN_INSTANCE_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <vector>
#include <string>

// -- Forward Declarations --
namespace pompeii::vulkan
{
	struct VulkanContext;
}

namespace pompeii::vulkan
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Instance
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Instance final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit Instance() = default;
		~Instance();

		Instance(const Instance& other) = delete;
		Instance(Instance&& other) noexcept = delete;
		Instance& operator=(const Instance& other) = delete;
		Instance& operator=(Instance&& other) noexcept = delete;

		//--------------------------------------------------
		//    Operation Overloads
		//--------------------------------------------------
		operator VkInstance() const;

	private:
		VkInstance	m_Instance	{ VK_NULL_HANDLE };
		friend class InstanceBuilder;
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  INSTANCE BUILDER
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class InstanceBuilder final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		InstanceBuilder();

		//--------------------------------------------------
		//    Builder
		//--------------------------------------------------
		InstanceBuilder& SetApplicationName(const std::string& name);
		InstanceBuilder& SetEngineName(const std::string& name);
		InstanceBuilder& SetAPIVersion(uint32_t apiVersion);
		InstanceBuilder& AddInstanceExtension(const char* extName);
		void Build(VulkanContext& context);

	private:
		VkApplicationInfo			m_AppInfo{};
		VkInstanceCreateInfo		m_CreateInfo{};
		std::vector<const char*>	m_vInstanceExtensions{};
	};
}

#endif // POMPEII_VULKAN_INSTANCE_H