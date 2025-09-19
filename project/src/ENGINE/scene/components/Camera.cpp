// -- Standard Library --
#include <algorithm>
#include <iostream>

// -- Keyboard --
#include "GLFW/glfw3.h"

// -- Pompeii Includes --
#include "Camera.h"
#include "ServiceLocator.h"
#include "SceneObject.h"
#include "Window.h"
#include "Timer.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  CLASS HERE	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::Camera::Camera(SceneObject& parent, const CameraSettings& settings, const Window* pWindow, bool mainCam)
	: Component(parent, "Camera")
	, m_Settings(settings)
	, m_ManualExposureSettings{ .aperture = 16.f, .shutterSpeed = 1.f / 100.f, .iso = 100.f}
	, m_AutoExposureSettings{ .minLogLum = -8.f, .logLumRange = 12.f }
	, m_pWindow(pWindow->GetHandle())
{
	if (mainCam)
		ServiceLocator::Get<RenderSystem>().SetMainCamera(*this);
}



//--------------------------------------------------
//    Loop
//--------------------------------------------------
void pompeii::Camera::Start()
{}
void pompeii::Camera::Update()
{
	if (!m_pWindow)
		std::cerr << "Window not valid for Camera!\n";

	const auto& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
		HandleAim();
	if (!io.WantCaptureKeyboard)
		HandleMovement();
}
void pompeii::Camera::OnInspectorDraw()
{
	// --- Movement ---
	ImGui::SeparatorText("Movement");
	ImGui::InputFloat("Speed", &m_Speed);
	ImGui::InputFloat("Sensitivity", &m_Sensitivity);

	// --- Projection ---
	ImGui::SeparatorText("Projection");
	if (ImGui::DragFloat("Field Of View", &m_Settings.fov, 0.1f, 1.f, 179.f))
		m_SettingsDirty = true;
	if (ImGui::DragFloat2("View Plane", &m_Settings.nearPlane, 0.01f))
		m_SettingsDirty = true;

	// --- Exposure ---
	ImGui::SeparatorText("Exposure");
	ImGui::Checkbox("Auto-Exposure", &m_AutoExposure);
	if (m_AutoExposure)
	{
		ImGui::DragFloat("Min Log Lum", &m_AutoExposureSettings.minLogLum, 0.01f);
		ImGui::DragFloat("Log Lum Range", &m_AutoExposureSettings.logLumRange, 0.01f);
	}
	else
	{
		ImGui::DragFloat("Aperture", &m_ManualExposureSettings.aperture, 0.01f, 0.1f, 32.0f);
		ImGui::DragFloat("ISO", &m_ManualExposureSettings.iso, 1.0f, 10.f, 12800.f);
		ImGui::DragFloat("ShutterSpeed", &m_ManualExposureSettings.shutterSpeed, 0.001f, 0.0001f, 10.f);
	}
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
// -- Settings --
void pompeii::Camera::ChangeSettings(const CameraSettings& settings) { m_Settings = settings; m_SettingsDirty = true; }
const pompeii::CameraSettings& pompeii::Camera::GetSettings() const	{ return m_Settings; }

const pompeii::ManualExposureSettings& pompeii::Camera::GetManualExposureSettings() const { return m_ManualExposureSettings; }
const pompeii::AutoExposureSettings& pompeii::Camera::GetAutoExposureSettings() const { return m_AutoExposureSettings; }
bool pompeii::Camera::IsAutoExposureEnabled() const { return m_AutoExposure; }

void pompeii::Camera::SetSpeed(float speed) { m_Speed = speed; }
void pompeii::Camera::SetSensitivity(float sensitivity) { m_Sensitivity = sensitivity; }

// -- Matrices --
glm::mat4 pompeii::Camera::GetViewMatrix() const
{
	return glm::inverse(GetTransform().GetMatrix());
}
glm::mat4 pompeii::Camera::GetProjectionMatrix()
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
void pompeii::Camera::HandleMovement() const
{
	float speed = m_Speed * Timer::GetDeltaSeconds();
	speed *= glfwGetKey(m_pWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 4.f : 1.f;


	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		GetTransform().Translate(GetTransform().GetForward() * speed);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		GetTransform().Translate(-GetTransform().GetForward() * speed);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		GetTransform().Translate(-GetTransform().GetRight() * speed);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		GetTransform().Translate(GetTransform().GetRight() * speed);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		GetTransform().Translate(glm::vec3(0, 1, 0) * speed);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		GetTransform().Translate(glm::vec3(0, -1, 0) * speed);
	}
}
void pompeii::Camera::HandleAim()
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

			glm::vec3 euler = GetTransform().GetEulerAngles();
			float pitch = euler.x + deltaY * m_Sensitivity;
			float yaw = euler.y + deltaX * m_Sensitivity;
			pitch = std::clamp(pitch, -89.f, 89.f);
			float roll = euler.z;
			GetTransform().SetEulerAngles({ pitch, yaw, roll });

			m_LastX = static_cast<float>(x);
			m_LastY = static_cast<float>(y);
		}
	}
	else
		m_IsDragging = false;
}
