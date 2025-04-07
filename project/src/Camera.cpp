// -- Keyboard --
#include "GLFW/glfw3.h"

// -- Camera --
#include "Camera.h"
#include "Window.h"

// -- Output --
#include <iostream>

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  CLASS HERE	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::Camera::Camera(const CameraSettings& settings, const Window* pWindow)
	: m_Settings(settings)
	, m_pWindow(pWindow->GetWindow())
{ }


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

// -- Settings --
void pom::Camera::ChangeSettings(const CameraSettings& settings)			{ m_Settings = settings; m_SettingsDirty = true; }
const pom::CameraSettings& pom::Camera::GetSettings()				const	{ return m_Settings; }

// -- Matrices --
glm::mat4 pom::Camera::GetViewMatrix()
{
	if (m_CameraDirty)
	{
		m_CameraDirty = false;
		m_ViewMatrix = lookAtLH(m_Position, m_Position + m_Forward, m_Up);
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
	constexpr float speed = 1.f;

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

	if (m_CameraDirty)
		UpdateCameraVectors();
}
void pom::Camera::HandleAim()
{
	static bool isDragging = false;
	static double lastX = 0.0;
	static double lastY = 0.0;
	if (glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		double x, y;
		glfwGetCursorPos(m_pWindow, &x, &y);

		if (!isDragging)
		{
			isDragging = true;
			lastX = x;
			lastY = y;
		}
		else
		{
			double deltaX = x - lastX;
			double deltaY = y - lastY;

			m_Forward.y -= deltaY * 0.01;
			m_Forward.x += deltaX * 0.01;
			m_CameraDirty = true;

			lastX = x;
			lastY = y;
		}
	}
	else
		isDragging = false;

	if (m_CameraDirty)
		UpdateCameraVectors();
}

void pom::Camera::UpdateCameraVectors()
{
	if (!m_CameraDirty)
		return;
	GetViewMatrix();

	glm::mat4 inverse = glm::inverse(m_ViewMatrix);
	m_Right		= inverse[0];
	m_Up		= inverse[1];
	m_Forward	= inverse[2];
}
