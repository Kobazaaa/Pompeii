#ifndef SAMPLER_H
#define SAMPLER_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Forward Declarations --
namespace pom { struct Context; }

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Sampler	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Sampler final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit Sampler() = default;
		~Sampler() = default;
		Sampler(const Sampler& other) = delete;
		Sampler(Sampler&& other) noexcept = delete;
		Sampler& operator=(const Sampler& other) = delete;
		Sampler& operator=(Sampler&& other) noexcept = delete;
		void Destroy(const Context& context) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkSampler& GetHandle() const;

	private:
		VkSampler m_Sampler;
		friend class SamplerBuilder;
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  SamplerBuilder	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class SamplerBuilder final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		SamplerBuilder();

		//--------------------------------------------------
		//    Builder
		//--------------------------------------------------
		// If not set, default of VK_FILTER_LINEAR is assumed
		SamplerBuilder& SetFilters(VkFilter magFilter, VkFilter minFilter);
		//! REQUIRED
		SamplerBuilder& SetAddressMode(VkSamplerAddressMode mode);
		// Anisotropy is VK_FALSE by default
		SamplerBuilder& EnableAnisotropy(float maxAnisotropy);
		// Black by default
		SamplerBuilder& SetBorderColor(VkBorderColor color);
		// Compare is VK_FALSE by default
		SamplerBuilder& EnableCompare(VkCompareOp op);
		// If not set, a default of VK_SAMPLER_MIPMAP_MODE_LINEAR is assumed
		SamplerBuilder& SetMipmapMode(VkSamplerMipmapMode mode);
		// All 0 by default
		SamplerBuilder& SetMipLevels(float bias, float min, float max);

		void Build(const Context& context, Sampler& sampler) const;

	private:
		VkSamplerCreateInfo m_CreateInfo{};
	};
}

#endif // SAMPLER_H