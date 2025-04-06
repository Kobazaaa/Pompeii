#include "DescriptorPool.h"
#include <stdexcept>


//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
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
pom::DescriptorPool& pom::DescriptorPool::AddPoolSizeLayout(const DescriptorSetLayout& layout)
{
	for (const auto& binding : layout.GetBindings())
	{
		AddPoolSize(binding.descriptorType, binding.descriptorCount * m_MaxSets);
	}
	return *this;
}

void pom::DescriptorPool::Create(const Device& device)
{
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(m_vPoolSizes.size());
	poolInfo.pPoolSizes = m_vPoolSizes.data();
	poolInfo.maxSets = m_MaxSets;
	poolInfo.flags = 0;

	if (vkCreateDescriptorPool(device.GetDevice(), &poolInfo, nullptr, &m_Pool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Descriptor Pool!");
}
void pom::DescriptorPool::Destroy(const Device& device) const
{
	vkDestroyDescriptorPool(device.GetDevice(), m_Pool, nullptr);
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
std::vector<pom::DescriptorSet> pom::DescriptorPool::AllocateSets(const Device& device, const DescriptorSetLayout& layout, uint32_t count) const
{
	std::vector<pom::DescriptorSet> results;
	std::vector<VkDescriptorSet> sets;
	sets.resize(count);
	results.resize(count);

	std::vector<VkDescriptorSetLayout> layouts(count, layout.GetLayout());
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_Pool;
	allocInfo.descriptorSetCount = count;
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(device.GetDevice(), &allocInfo, sets.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate Descriptor Sets!");

	for (uint32_t index{}; index < count; ++index)
	{
		results[index].m_DescriptorSet = sets[index];
		results[index].m_Layout = layout;
	}

	return results;
}
const VkDescriptorPool& pom::DescriptorPool::GetPool() const { return m_Pool; }
