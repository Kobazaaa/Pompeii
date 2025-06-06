#ifndef CAMERA_H
#define CAMERA_H

// -- Defines --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS

// -- Math Includes --
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

// -- Forward Declarations --
namespace pom { class Window; }

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  CameraSettings	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct CameraSettings
	{
		float fov;
		float aspectRatio;
		float nearPlane;
		float farPlane;
	};
	struct ExposureSettings
	{
		float aperture;
		float shutterSpeed;
		float iso;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Camera	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Camera final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		Camera() = default;
		explicit Camera(const CameraSettings& settings, const ExposureSettings& exposureSettings, const Window* pWindow);

		//--------------------------------------------------
		//    Update
		//--------------------------------------------------
		void Update();

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		// -- Data --
		glm::vec3 GetPosition() const;

		// -- Settings --
		void ChangeSettings(const CameraSettings& settings);
		const CameraSettings& GetSettings() const;
		void ChangeExposureSettings(const ExposureSettings& settings);
		const ExposureSettings& GetExposureSettings() const;

		void SetSpeed(float speed);
		void SetSensitivity(float sensitivity);

		// -- Matrices --
		glm::mat4 GetViewMatrix();
		glm::mat4 GetProjectionMatrix();

	private:
		// -- Helpers --
		void HandleMovement();
		void HandleAim();

		void UpdateCameraVectors();

		// -- Data --
		glm::vec3 m_Position				{ 0.f, 0.f, 0.f };
		glm::vec3 m_Forward					{ 0.f, 0.f, 1.f };
		glm::vec3 m_Right					{ 1.f, 0.f, 1.f };
		glm::vec3 m_Up						{ 0.f, 1.f, 0.f };

		glm::mat4 m_ViewMatrix				{ };
		glm::mat4 m_ProjectionMatrix		{ };

		// -- Settings --
		CameraSettings	 m_Settings			{ };
		ExposureSettings m_ExposureSettings	{ };

		float m_Pitch						{ };
		float m_Yaw							{ };
		float m_Roll						{ };

		float m_Speed						{ 1.f };
		float m_Sensitivity					{ 0.1f };

		bool m_IsDragging					{ false };
		float m_LastX						{ 0.f };
		float m_LastY						{ 0.f };

		// -- Dirty Flags
		bool m_CameraDirty					{ true };
		bool m_SettingsDirty				{ true };

		// -- Window --
		GLFWwindow* m_pWindow				{ nullptr };
	};
}

#endif // CAMERA_H