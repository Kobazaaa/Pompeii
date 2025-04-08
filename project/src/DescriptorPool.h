#ifndef DESCRIPTOR_POOL_H
#define DESCRIPTOR_POOL_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <vector>

// -- Forward Declarations --
namespace pom
{
	class DescriptorSetLayout;
	class DescriptorSet;
	struct Context;
}


namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  DescriptorPool	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class DescriptorPool final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		DescriptorPool() = default;
		DescriptorPool& SetMaxSets(uint32_t count);
		DescriptorPool& AddPoolSize(VkDescriptorType type, uint32_t count);
		DescriptorPool& AddPoolSizeLayout(const DescriptorSetLayout& layout);
		void Create(const Context& context);
		void Destroy(const Context& context) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		std::vector<DescriptorSet> AllocateSets(const Context& context, const DescriptorSetLayout& layout, uint32_t count) const;
		const VkDescriptorPool& GetHandle() const;

	private:
		VkDescriptorPool m_Pool{ VK_NULL_HANDLE };
		std::vector<VkDescriptorPoolSize> m_vPoolSizes{};
		uint32_t m_MaxSets;
	};
}

#endif // DESCRIPTOR_POOL_H