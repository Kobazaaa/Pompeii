#ifndef SCENE_H
#define SCENE_H

// -- Pom Includes --
#include "Camera.h"
#include "Light.h"
#include "Model.h"

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Base Scene	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Scene
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit Scene() = default;
		virtual ~Scene() = default;

		Scene(const Scene& other) = delete;
		Scene(Scene&& other) noexcept = delete;
		Scene& operator=(const Scene& other) = delete;
		Scene& operator=(Scene&& other) noexcept = delete;

		virtual void Initialize() = 0;
		virtual void AllocateGPU(const Context& context, CommandPool& cmdPool, bool keepHostData = false);
		virtual void Destroy(const Context& context);


		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		const std::vector<Model>& GetModels() const;
		Model& AddModel(const std::string& path);
		uint32_t GetImageCount() const;

		std::vector<Light>& GetLights();
		std::vector<GPULight> GetLightsGPU();
		uint32_t GetLightsCount() const;
		Light& AddLight(const Light& light);

	private:
		std::vector<Model> m_vModels;
		std::vector<Light> m_vLights;
		std::vector<GPULight> m_vGPULights;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Sponza Scene	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class SponzaScene final : public Scene
	{
	public:
		explicit SponzaScene() = default;
		void Initialize() override;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  FlightHelmet Scene	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class FlightHelmetScene final : public Scene
	{
	public:
		explicit FlightHelmetScene() = default;
		void Initialize() override;
	};
}

#endif // SCENE_H