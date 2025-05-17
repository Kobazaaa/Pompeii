
// -- Pompeii Includes --
#include "Scene.h"

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::Scene::Load(const std::string& path)
{
	model.LoadModel(path);

	vLights.emplace_back(DirectionalLight
		{ /* direction */	{ 0.577f, -0.577f, 0.577f },
		/* color */		{ 1.f, 1.f, 1.f },
		/* intensity */	1.f,
		/* size */		{ 5000.f, 5000.f },
		/* near-far */	{ 0.1f, 11000.f },
		/* distance */	10000.f });
}
void pom::Scene::Allocate(const Context& context, CommandPool& cmdPool, bool keepHostData)
{
	model.AllocateResources(context, cmdPool, keepHostData);
}
void pom::Scene::Destroy()
{
	model.Destroy();
}
