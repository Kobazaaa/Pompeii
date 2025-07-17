#ifndef DESCRIPTOR_POOL_H
#define DESCRIPTOR_POOL_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <vector>

// -- Forward Declarations --
namespace pompeii
{
	class DescriptorSetLayout;
	class DescriptorSet;
	struct Context;
}

namespace pompeii
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
		explicit DescriptorPool() = default;
		~DescriptorPool() = default;
		DescriptorPool(const DescriptorPool& other) = delete;
		DescriptorPool(DescriptorPool&& other) noexcept = delete;
		DescriptorPool& operator=(const DescriptorPool& other) = delete;
		DescriptorPool& operator=(DescriptorPool&& other) noexcept = delete;

		DescriptorPool& SetDebugName(const char* name);
		DescriptorPool& SetMaxSets(uint32_t count);
		DescriptorPool& AddPoolSize(VkDescriptorType type, uint32_t count);
		DescriptorPool& AddFlags(VkDescriptorPoolCreateFlags flags);
		void Create(const Context& context);
		void Destroy(const Context& context) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		std::vector<DescriptorSet> AllocateSets(const Context& context, const DescriptorSetLayout& layout, uint32_t count, const char* name = nullptr, const void* pNext = nullptr) const;
		const VkDescriptorPool& GetHandle() const;

	private:
		VkDescriptorPool m_Pool{ VK_NULL_HANDLE };
		std::vector<VkDescriptorPoolSize> m_vPoolSizes{};
		uint32_t m_MaxSets;
		VkDescriptorPoolCreateFlags m_CreateFlags{};
		const char* m_pName{};
	};
}

#endif // DESCRIPTOR_POOL_H