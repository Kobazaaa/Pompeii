#ifndef DESCRIPTOR_POOL_H
#define DESCRIPTOR_POOL_H

#include <vector>
#include "Device.h"
#include "DescriptorSet.h"

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
		void Create(const Device& device);
		void Destroy(const Device& device) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		std::vector<DescriptorSet> AllocateSets(const Device& device, const DescriptorSetLayout& layout, uint32_t count) const;
		const VkDescriptorPool& GetPool() const;

	private:
		VkDescriptorPool m_Pool{ VK_NULL_HANDLE };
		std::vector<VkDescriptorPoolSize> m_vPoolSizes{};
		uint32_t m_MaxSets;
	};
}

#endif // DESCRIPTOR_POOL_H