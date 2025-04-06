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
		Instance() = default;
		void Destroy() const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkInstance& GetInstance() const;

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
		void Build(Instance& instance);
	private:
		static std::vector<const char*> GetRequiredExtensions();

		VkApplicationInfo		m_AppInfo{};
		VkInstanceCreateInfo	m_CreateInfo{};
	};
}

#endif // INSTANCE_H