// -- Standard Library --
#include <algorithm>
#include <iostream>

// -- Keyboard --
#include "GLFW/glfw3.h"

// -- Pompeii Includes --
#include "Camera.h"
#include "Window.h"
#include "Timer.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  CLASS HERE	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::Camera::Camera(const CameraSettings& settings, const ExposureSettings& exposureSettings, const Window* pWindow)
	: m_Settings(settings)
	, m_ExposureSettings(exposureSettings)
	, m_pWindow(pWindow->GetHandle())
{}


//--------------------------------------------------
//    Update
//--------------------------------------------------
void pom::Camera::Update()
{
	if (!m_pWindow)
		std::cerr << "Window not valid for Camera!\n";

	HandleMovement();
	HandleAim();
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
// -- Data --
glm::vec3 pom::Camera::GetPosition()										const	{ return m_Position; }

// -- Settings --
void pom::Camera::ChangeSettings(const CameraSettings& settings)					{ m_Settings = settings; m_SettingsDirty = true; }
const pom::CameraSettings& pom::Camera::GetSettings()						const	{ return m_Settings; }
void pom::Camera::ChangeExposureSettings(const ExposureSettings& settings)			{ m_ExposureSettings = settings; }
const pom::ExposureSettings& pom::Camera::GetExposureSettings()				const	{ return m_ExposureSettings; }

void pom::Camera::SetSpeed(float speed)												{ m_Speed = speed; }
void pom::Camera::SetSensitivity(float sensitivity)									{ m_Sensitivity = sensitivity; }

// -- Matrices --
glm::mat4 pom::Camera::GetViewMatrix()
{
	if (m_CameraDirty)
	{
		m_CameraDirty = false;
		m_ViewMatrix = lookAtLH(m_Position, m_Position + m_Forward, glm::vec3(0.f, 1.f, 0.f));
	}
	return m_ViewMatrix;
}
glm::mat4 pom::Camera::GetProjectionMatrix()
{
	if (m_SettingsDirty)
	{
		m_SettingsDirty = false;
		m_ProjectionMatrix = glm::perspectiveLH(glm::radians(m_Settings.fov), m_Settings.aspectRatio,
															 m_Settings.nearPlane, m_Settings.farPlane);
		m_ProjectionMatrix[1][1] *= -1;
	}
	return m_ProjectionMatrix;
}


//--------------------------------------------------
//    Helpers
//--------------------------------------------------
void pom::Camera::HandleMovement()
{
	float speed = m_Speed * Timer::GetDeltaSeconds();
	speed *= glfwGetKey(m_pWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 4.f : 1.f;


	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		m_CameraDirty = true;
		m_Position += m_Forward * speed;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		m_CameraDirty = true;
		m_Position -= m_Forward * speed;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		m_CameraDirty = true;
		m_Position -= m_Right * speed;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		m_CameraDirty = true;
		m_Position += m_Right * speed;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		m_CameraDirty = true;
		m_Position.y += speed;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		m_CameraDirty = true;
		m_Position.y -= speed;
	}

	if (m_CameraDirty)
		UpdateCameraVectors();
}
void pom::Camera::HandleAim()
{
	if (glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		double x, y;
		glfwGetCursorPos(m_pWindow, &x, &y);

		if (!m_IsDragging)
		{
			m_IsDragging = true;
			m_LastX = static_cast<float>(x);
			m_LastY = static_cast<float>(y);
		}
		else
		{
			const float deltaX = static_cast<float>(x) - m_LastX;
			const float deltaY = static_cast<float>(y) - m_LastY;

			m_Pitch += deltaY * m_Sensitivity;
			m_Yaw += deltaX * m_Sensitivity;
			m_Pitch = std::clamp(m_Pitch, -89.f, 89.f);

			m_CameraDirty = true;
			m_LastX = static_cast<float>(x);
			m_LastY = static_cast<float>(y);
		}
	}
	else
		m_IsDragging = false;

	if (m_CameraDirty)
		UpdateCameraVectors();
}

void pom::Camera::UpdateCameraVectors()
{
	if (!m_CameraDirty)
		return;

	glm::mat4 rotation = glm::mat4(1.0f);
	rotation = rotate(rotation, glm::radians(m_Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
	rotation = rotate(rotation, glm::radians(m_Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	rotation = rotate(rotation, glm::radians(m_Roll), glm::vec3(0.0f, 0.0f, 1.0f));
	m_Forward = rotation * glm::vec4(0.f, 0.f, 1.f, 0.f);

	GetViewMatrix();

	glm::mat4 inverse = glm::inverse(m_ViewMatrix);
	m_Right		= inverse[0];
	m_Up		= inverse[1];
	m_Forward	= inverse[2];
}
