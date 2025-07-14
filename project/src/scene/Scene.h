#ifndef SCENE_H
#define SCENE_H

// -- Defines --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS

// -- Standard Library --
#include <memory>

// -- Pom Includes --
#include "SceneObject.h"
#include "EnvironmentMap.h"
#include "Light.h"
#include "Model.h"
#include "Camera.h"

namespace pompeii
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
		explicit Scene(std::string sceneName);
		virtual ~Scene() = default; //todo remove virtual when class is made final

		Scene(const Scene& other) = delete;
		Scene(Scene&& other) = delete;
		Scene& operator=(const Scene& other) = delete;
		Scene& operator=(Scene&& other) = delete;

		//--------------------------------------------------
		//    Adding & Removing SceneObject
		//--------------------------------------------------
		[[nodiscard]] SceneObject& AddEmpty(const std::string& sceneName = "SceneObject");

		//--------------------------------------------------
		//    Loop
		//--------------------------------------------------
		void Start() const;
		void Update();
		void OnImGuiRender() const;

		//--------------------------------------------------
		//    Accessors
		//--------------------------------------------------
		std::string name{ "EmptyScene" };
		std::vector<SceneObject*> GetObjectsByName(const std::string& objectName) const;

	private:
		void CleanupDeletedObjects();
		void AddPendingObjects();

		std::vector<std::unique_ptr<SceneObject>> m_vObjects{};
		std::vector<std::unique_ptr<SceneObject>> m_vPendingObjects{};


































	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit Scene() = default;

		virtual void Initialize() {};
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