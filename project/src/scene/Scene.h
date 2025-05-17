#ifndef SCENE_H
#define SCENE_H

// -- Pom Includes --
#include "DirectionalLight.h"
#include "Model.h"

namespace pom
{
	struct Scene
	{
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		Scene() = default;
		void Load(const std::string& path);
		void Allocate(const Context& context, CommandPool& cmdPool, bool keepHostData = false);
		void Destroy();


		Model model;
		std::vector<DirectionalLight> vLights;
	};
}

#endif // SCENE_H