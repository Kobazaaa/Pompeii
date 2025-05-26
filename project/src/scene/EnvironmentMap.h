#ifndef ENVIRONMENT_MAP_H
#define ENVIRONMENT_MAP_H

// -- Math Includes --
#include "glm/glm.hpp"

// -- Pompeii Includes --
#include "Image.h"
#include "Sampler.h"

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  EnvironmentMap	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class EnvironmentMap final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit EnvironmentMap() = default;
		void Destroy(const Context& context);

		//--------------------------------------------------
		//    Initializers
		//--------------------------------------------------
		EnvironmentMap& CreateSampler(const Context& context);
		EnvironmentMap& CreateSkyboxCube(const Context& context, const std::string& path, uint32_t size = 1024);
		EnvironmentMap& CreateDiffIrradianceMap(const Context& context, uint32_t size = 64);
		EnvironmentMap& CreateSpecIrradianceMap(const Context& context, uint32_t size = 128);
		EnvironmentMap& CreateBRDFLut(const Context& context, uint32_t size = 512);

		//--------------------------------------------------
		//    Accessors
		//--------------------------------------------------
		const Sampler& GetSampler() const;
		const Image& GetSkybox() const;
		const Image& GetDiffuseIrradianceMap() const;
		const Image& GetSpecularIrradianceMap() const;
		const Image& GetBRDFLut() const;

	private:
		// -- Helpers --
		void RenderToCubeMap(const Context& context,
							 const std::string& vert, const std::string& frag,
							 Image& inImage, const ImageView& inView, const Sampler& inSampler,
							 Image& outImage, std::array<std::vector<ImageView>, 6>& outViews, uint32_t size);

		// -- Data --
		Sampler m_Sampler{};
		Image m_Skybox;
		Image m_DiffuseIrradiance;
		Image m_SpecularIrradiance;
		Image m_BRDFLut;
	};
}

#endif // ENVIRONMENT_MAP_H
