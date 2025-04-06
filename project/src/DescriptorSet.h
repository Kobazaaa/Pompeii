#ifndef DESCRIPTOR_SET_H
#define DESCRIPTOR_SET_H

#include <vector>
#include "Device.h"

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  DescriptorSetLayout	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class DescriptorSetLayout final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		DescriptorSetLayout() = default;
		void Destroy(const Device& device) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkDescriptorSetLayout& GetLayout() const;
	private:
		VkDescriptorSetLayout m_Layout;
		friend class DescriptorSetLayoutBuilder;
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  DescriptorSetLayoutBuilder	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class DescriptorSetLayoutBuilder final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		DescriptorSetLayoutBuilder() = default;

		//--------------------------------------------------
		//    Builder
		//--------------------------------------------------
		DescriptorSetLayoutBuilder& NewLayoutBinding();
		//! REQUIRED
		DescriptorSetLayoutBuilder& SetType(VkDescriptorType type);
		// If not called, assumes 1 b default
		DescriptorSetLayoutBuilder& SetCount(uint32_t count);
		//! REQUIRED
		DescriptorSetLayoutBuilder& SetShaderStages(VkShaderStageFlags flags);

		void Build(const Device& device, DescriptorSetLayout& descriptorSetLayout) const;

	private:
		std::vector<VkDescriptorSetLayoutBinding> m_vLayoutBindings;
	};
}

#endif // DESCRIPTOR_SET_H