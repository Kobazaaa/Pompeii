#include "Sampler.h"
#include <stdexcept>

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Sampler	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::Sampler::Destroy(const Device& device)	const { vkDestroySampler(device.GetDevice(), m_Sampler, nullptr); }

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkSampler& pom::Sampler::GetSampler()			const { return m_Sampler; }



//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  SamplerBuilder	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::SamplerBuilder::SamplerBuilder()
{
	m_CreateInfo = {};
	m_CreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;				// CAN'T CHANGE
	m_CreateInfo.magFilter = VK_FILTER_LINEAR;								//? CAN CHANGE
	m_CreateInfo.minFilter = VK_FILTER_LINEAR;								//? CAN CHANGE
	m_CreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;				//! REQUIRED CHANGE
	m_CreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;				//! REQUIRED CHANGE
	m_CreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;				//! REQUIRED CHANGE
	m_CreateInfo.anisotropyEnable = VK_FALSE;								//? CAN CHANGE
	m_CreateInfo.maxAnisotropy = 0;											//? CAN CHANGE
	m_CreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;			//? CAN CHANGE
	m_CreateInfo.unnormalizedCoordinates = VK_FALSE;						// CAN'T CHANGE
	m_CreateInfo.compareEnable = VK_FALSE;									//? CAN CHANGE
	m_CreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;							//? CAN CHANGE
	m_CreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;				//? CAN CHANGE
	m_CreateInfo.mipLodBias = 0.0f;											//? CAN CHANGE
	m_CreateInfo.minLod = 0.0f;												//? CAN CHANGE
	m_CreateInfo.maxLod = 0.0f;												//? CAN CHANGE
}


//--------------------------------------------------
//    Builder
//--------------------------------------------------
pom::SamplerBuilder& pom::SamplerBuilder::SetFilters(VkFilter magFilter, VkFilter minFilter)
{
	m_CreateInfo.magFilter = magFilter;
	m_CreateInfo.minFilter = minFilter;
	return *this;
}
pom::SamplerBuilder& pom::SamplerBuilder::SetAddressMode(VkSamplerAddressMode mode)
{
	m_CreateInfo.addressModeU = mode;
	m_CreateInfo.addressModeV = mode;
	m_CreateInfo.addressModeW = mode;
	return *this;
}
pom::SamplerBuilder& pom::SamplerBuilder::EnableAnisotropy(float maxAnisotropy)
{
	m_CreateInfo.anisotropyEnable = VK_TRUE;
	m_CreateInfo.maxAnisotropy = maxAnisotropy;
	return *this;
}
pom::SamplerBuilder& pom::SamplerBuilder::SetBorderColor(VkBorderColor color)
{
	m_CreateInfo.borderColor = color;
	return *this;
}
pom::SamplerBuilder& pom::SamplerBuilder::EnableCompare(VkCompareOp op)
{
	m_CreateInfo.compareEnable = VK_TRUE;
	m_CreateInfo.compareOp = op;
	return *this;
}
pom::SamplerBuilder& pom::SamplerBuilder::SetMipmapMode(VkSamplerMipmapMode mode)
{
	m_CreateInfo.mipmapMode = mode;
	return *this;
}
pom::SamplerBuilder& pom::SamplerBuilder::SetMipLevels(float bias, float min, float max)
{
	m_CreateInfo.mipLodBias = bias;
	m_CreateInfo.minLod = min;
	m_CreateInfo.maxLod = max;
	return *this;
}

void pom::SamplerBuilder::Build(const Device& device, Sampler& sampler)
{
	if (vkCreateSampler(device.GetDevice(), &m_CreateInfo, nullptr, &sampler.m_Sampler) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Texture Sampler!");
}
