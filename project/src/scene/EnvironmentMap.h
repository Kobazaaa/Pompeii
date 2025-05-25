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

		//--------------------------------------------------
		//    Accessors
		//--------------------------------------------------
		const Sampler& GetSampler() const;
		const Image& GetSkybox() const;

	private:
		// -- Helpers --
		void RenderToCubeMap(const Context& context,
							 const std::string& vert, const std::string& frag,
							 Image& inImage, const ImageView& inView, const Sampler& inSampler,
							 Image& outImage, std::array<std::vector<ImageView>, 6>& outViews, uint32_t size);

		// -- Data --
		Sampler m_Sampler{};
		Image m_Skybox;
	};
}

#endif // ENVIRONMENT_MAP_H
