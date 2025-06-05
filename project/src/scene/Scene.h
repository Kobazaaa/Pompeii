#ifndef SCENE_H
#define SCENE_H

// -- Defines --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS

// -- Pom Includes --
#include "Camera.h"
#include "EnvironmentMap.h"
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
		virtual void AllocateGPU(const Context& context, bool keepHostData = false);
		virtual void Destroy(const Context& context);


		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		// -- Models --
		const std::vector<Model>& GetModels() const;
		Model& AddModel(const std::string& path);
		uint32_t GetImageCount() const;

		// -- Lights --
		std::vector<Light>& GetLights();
		std::vector<GPULight> GetLightsGPU();
		std::vector<glm::mat4> GetLightMatrices();
		uint32_t GetLightsCount() const;
		Light& AddLight(Light&& light);
		void PopLight();

		void CalculateLightMatrices();
		void GenerateDepthMaps(const Context& context, uint32_t size = 2048);

		// -- Environment Map --
		const EnvironmentMap& GetEnvironmentMap() const;
	protected:
		void SetEnvironmentMap(const std::string& path);

	private:
		std::vector<Model> m_vModels;
		std::vector<Light> m_vLights;
		AABB m_AABB;

		std::string m_EnvMapPath{"textures/circus_arena_4k.hdr"};
		EnvironmentMap m_EnvironmentMap;
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

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Spheres Scene	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class SpheresScene final : public Scene
	{
	public:
		explicit SpheresScene() = default;
		void Initialize() override;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  A Beautiful Game Scene	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class BeautifulGameScene final : public Scene
	{
	public:
		explicit BeautifulGameScene() = default;
		void Initialize() override;
	};
}

#endif // SCENE_H