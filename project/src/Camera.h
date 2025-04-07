#ifndef CAMERA_H
#define CAMERA_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace pom
{
	class Window;
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
		// -- Settings --
		void ChangeSettings(const CameraSettings& settings);
		const CameraSettings& GetSettings() const;

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

		// -- Dirty Flags
		bool m_CameraDirty				{ true };
		bool m_SettingsDirty			{ true };

		// -- Window --
		GLFWwindow* m_pWindow			{ nullptr };
	};
}

#endif // CAMERA_H