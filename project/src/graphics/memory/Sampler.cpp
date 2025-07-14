// -- Standard Library --
#include <stdexcept>

//-- Pompeii Includes --
#include "Sampler.h"
#include "Context.h"

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Sampler	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pompeii::Sampler::Destroy(const Context& context)	const { vkDestroySampler(context.device.GetHandle(), m_Sampler, nullptr); }

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkSampler& pompeii::Sampler::GetHandle()			const { return m_Sampler; }



//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  SamplerBuilder	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::SamplerBuilder::SamplerBuilder()
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
	m_CreateInfo.maxLod = VK_LOD_CLAMP_NONE;								//? CAN CHANGE
}


//--------------------------------------------------
//    Builder
//--------------------------------------------------
pompeii::SamplerBuilder& pompeii::SamplerBuilder::SetFilters(VkFilter magFilter, VkFilter minFilter)
{
	m_CreateInfo.magFilter = magFilter;
	m_CreateInfo.minFilter = minFilter;
	return *this;
}
pompeii::SamplerBuilder& pompeii::SamplerBuilder::SetAddressMode(VkSamplerAddressMode mode)
{
	m_CreateInfo.addressModeU = mode;
	m_CreateInfo.addressModeV = mode;
	m_CreateInfo.addressModeW = mode;
	return *this;
}
pompeii::SamplerBuilder& pompeii::SamplerBuilder::EnableAnisotropy(float maxAnisotropy)
{
	m_CreateInfo.anisotropyEnable = VK_TRUE;
	m_CreateInfo.maxAnisotropy = maxAnisotropy;
	return *this;
}
pompeii::SamplerBuilder& pompeii::SamplerBuilder::SetBorderColor(VkBorderColor color)
{
	m_CreateInfo.borderColor = color;
	return *this;
}
pompeii::SamplerBuilder& pompeii::SamplerBuilder::EnableCompare(VkCompareOp op)
{
	m_CreateInfo.compareEnable = VK_TRUE;
	m_CreateInfo.compareOp = op;
	return *this;
}
pompeii::SamplerBuilder& pompeii::SamplerBuilder::SetMipmapMode(VkSamplerMipmapMode mode)
{
	m_CreateInfo.mipmapMode = mode;
	return *this;
}
pompeii::SamplerBuilder& pompeii::SamplerBuilder::SetMipLevels(float bias, float min, float max)
{
	m_CreateInfo.mipLodBias = bias;
	m_CreateInfo.minLod = min;
	m_CreateInfo.maxLod = max;
	return *this;
}

void pompeii::SamplerBuilder::Build(const Context& context, Sampler& sampler) const
{
	if (vkCreateSampler(context.device.GetHandle(), &m_CreateInfo, nullptr, &sampler.m_Sampler) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Texture Sampler!");
}
