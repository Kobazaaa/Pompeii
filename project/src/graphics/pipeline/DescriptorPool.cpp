// -- Standard Library --
#include <stdexcept>

// -- Pompeii Includes --
#include "DescriptorPool.h"
#include "Context.h"
#include "Debugger.h"
#include "DescriptorSet.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  DescriptorPool	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::DescriptorPool& pom::DescriptorPool::SetDebugName(const char* name)
{
	m_pName = name;
	return *this;
}
pom::DescriptorPool& pom::DescriptorPool::SetMaxSets(uint32_t count)
{
	m_MaxSets = count;
	return *this;
}
pom::DescriptorPool& pom::DescriptorPool::AddPoolSize(VkDescriptorType type, uint32_t count)
{
	m_vPoolSizes.emplace_back(type, count);
	return *this;
}
pom::DescriptorPool& pom::DescriptorPool::AddFlags(VkDescriptorPoolCreateFlags flags)
{
	m_CreateFlags |= flags;
	return *this;
}

void pom::DescriptorPool::Create(const Context& context)
{
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(m_vPoolSizes.size());
	poolInfo.pPoolSizes = m_vPoolSizes.data();
	poolInfo.maxSets = m_MaxSets;
	poolInfo.flags = m_CreateFlags;

	if (vkCreateDescriptorPool(context.device.GetHandle(), &poolInfo, nullptr, &m_Pool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Descriptor Pool!");

	if (m_pName)
	{
		Debugger::SetDebugObjectName(reinterpret_cast<uint64_t>(m_Pool), VK_OBJECT_TYPE_DESCRIPTOR_POOL, m_pName);
	}

	m_pName = nullptr;
	m_CreateFlags = {};
}
void pom::DescriptorPool::Destroy(const Context& context) const
{
	vkDestroyDescriptorPool(context.device.GetHandle(), m_Pool, nullptr);
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
std::vector<pom::DescriptorSet> pom::DescriptorPool::AllocateSets(const Context& context, const DescriptorSetLayout& layout, uint32_t count, const char* name, const void* pNext) const
{
	std::vector<pom::DescriptorSet> results;
	std::vector<VkDescriptorSet> sets;
	sets.resize(count);
	results.resize(count);

	std::vector<VkDescriptorSetLayout> layouts(count, layout.GetHandle());
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_Pool;
	allocInfo.descriptorSetCount = count;
	allocInfo.pSetLayouts = layouts.data();
	allocInfo.pNext = pNext;

	if (vkAllocateDescriptorSets(context.device.GetHandle(), &allocInfo, sets.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate Descriptor Sets!");

	for (uint32_t index{}; index < count; ++index)
	{
		results[index].m_DescriptorSet = sets[index];
		results[index].m_Layout = layout;

		if (name)
		{
			std::string str{ name };
			str += std::to_string(index);
			Debugger::SetDebugObjectName(reinterpret_cast<uint64_t>(sets[index]), VK_OBJECT_TYPE_DESCRIPTOR_SET, str);
		}
	}

	return results;
}
const VkDescriptorPool& pom::DescriptorPool::GetHandle() const { return m_Pool; }
