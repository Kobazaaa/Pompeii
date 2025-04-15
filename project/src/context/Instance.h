#ifndef INSTANCE_H
#define INSTANCE_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <string>
#include <vector>

// -- Forward Declarations --
namespace pom { struct Context; }


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
		const VkInstance& GetHandle() const;

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
		void Build(Context& context);

	private:
		void GetRequiredExtensions();

		VkApplicationInfo			m_AppInfo{};
		VkInstanceCreateInfo		m_CreateInfo{};
		std::vector<const char*>	m_vInstanceExtensions{};
	};
}

#endif // INSTANCE_H