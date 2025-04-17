#ifndef CAMERA_H
#define CAMERA_H

// -- Math Includes --
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
		explicit Camera(const CameraSettings& settings, const Window* pWindow);

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
		glm::vec3 m_Position			{ 0.f, 0.f, 0.f };
		glm::vec3 m_Forward				{ 0.f, 0.f, 1.f };
		glm::vec3 m_Right				{ 1.f, 0.f, 1.f };
		glm::vec3 m_Up					{ 0.f, 1.f, 0.f };

		glm::mat4 m_ViewMatrix			{ };
		glm::mat4 m_ProjectionMatrix	{ };

		// -- Settings --
		CameraSettings m_Settings		{ };

		float m_Pitch					{ };
		float m_Yaw						{ };
		float m_Roll					{ };

		float m_Speed					{ 100.f };
		float m_Sensitivity				{ 0.1f };

		bool m_IsDragging				{ false };
		float m_LastX					{ 0.f };
		float m_LastY					{ 0.f };

		// -- Dirty Flags
		bool m_CameraDirty				{ true };
		bool m_SettingsDirty			{ true };

		// -- Window --
		GLFWwindow* m_pWindow			{ nullptr };
	};
}

#endif // CAMERA_H