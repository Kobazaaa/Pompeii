#ifndef INSTANCE_H
#define INSTANCE_H

#include <string>
#include <vulkan/vulkan.h>

#include "Debugger.h"

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  INSTANCE	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Instance final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit Instance(VkInstance instance);
		~Instance();

		Instance(const Instance& other) = delete;
		Instance(Instance&& other) noexcept = delete;
		Instance& operator=(const Instance& other) = delete;
		Instance& operator=(Instance&& other) noexcept = delete;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		VkInstance& GetInstance();
	private:

		VkInstance	m_Instance	{ VK_NULL_HANDLE };
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
		InstanceBuilder() = default;


		//--------------------------------------------------
		//    Builder
		//--------------------------------------------------
		InstanceBuilder& SetApplicationName(const std::string& name);
		InstanceBuilder& SetEngineName(const std::string& name);
		Instance& Build(Instance& instance);
	private:
		std::vector<const char*> GetRequiredExtensions();

		VkApplicationInfo		m_AppInfo{};
		VkInstanceCreateInfo	m_CreateInfo{};
	};
}

#endif // INSTANCE_H