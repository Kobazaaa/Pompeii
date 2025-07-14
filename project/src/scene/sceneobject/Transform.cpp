// -- Pompeii Includes --
#include "Transform.h"

// -- Math Includes --
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtx/matrix_decompose.hpp>


//--------------------------------------------------
//    Constructor
//--------------------------------------------------
pompeii::Transform::Transform(SceneObject* pOwner)
	: m_pOwnerObject{ pOwner }
{}


//--------------------------------------------------
//    Parent-Child
//--------------------------------------------------
pompeii::SceneObject* pompeii::Transform::GetOwner() const { return m_pOwnerObject; }
pompeii::Transform* pompeii::Transform::GetParent() const { return m_pParent; }
void pompeii::Transform::SetParent(Transform* parent, bool keepWorldPosition)
{
	// avoid unnecessary reparenting
	if (IsChild(parent) || parent == this || m_pParent == parent)
		return;

	// calculate new transforms
	if (keepWorldPosition)
	{
		if (parent != nullptr)
		{
			const glm::mat4 localMatrix = glm::inverse(parent->GetMatrix()) * GetMatrix();
			DecomposeMatrixToLocals(localMatrix);
		}
		else
		{
			SetLocalPosition(GetPosition());
			SetLocalScale(GetScale());
			SetLocalEulerAngles(GetEulerAngles());
		}
	}

	// reparent
	if (m_pParent) m_pParent->RemoveChild(this);
	m_pParent = parent;
	if (m_pParent) m_pParent->AddChild(this);
}

bool pompeii::Transform::IsChild(const Transform* child) const
{
	return std::ranges::find(m_vChildren, child) != m_vChildren.end();
}
int pompeii::Transform::GetChildCount() const
{
	return static_cast<int>(m_vChildren.size());
}
const std::vector<pompeii::Transform*>& pompeii::Transform::GetAllChildren() const
{
	return m_vChildren;
}

void pompeii::Transform::AddChild(Transform* child)
{
	// If the child already exists in the container, don't add it again!
	if (std::ranges::find(m_vChildren, child) != m_vChildren.end())
		return;
	m_vChildren.push_back(child);
}
void pompeii::Transform::RemoveChild(const Transform* child)
{
	std::erase_if(m_vChildren, [&](const Transform* pChild)
		{
			return pChild == child;
		});
}


//--------------------------------------------------
//    Transformation
//--------------------------------------------------
const glm::vec3& pompeii::Transform::GetPosition()						{ if (m_DirtyPosition) RecalculatePosition(); return m_Position; }
const glm::vec3& pompeii::Transform::GetLocalPosition()		const		{ return m_LocalPosition; }
void pompeii::Transform::SetLocalPosition(const glm::vec3& pos)			{ m_LocalPosition = pos; SetPositionDirty(); }

const glm::vec3& pompeii::Transform::GetEulerAngles()					{ if (m_DirtyEulerAngles) RecalculateEulerAngles(); return m_EulerAngles; }
const glm::vec3& pompeii::Transform::GetLocalEulerAngles()	const		{ return m_LocalEulerAngles; }
void pompeii::Transform::SetLocalEulerAngles(const glm::vec3& euler)	{ m_LocalEulerAngles = euler; SetEulerAnglesDirty(); }

const glm::vec3& pompeii::Transform::GetScale()							{ if (m_DirtyScale) RecalculateScale(); return m_Scale; }
const glm::vec3& pompeii::Transform::GetLocalScale()		const		{ return m_LocalScale; }
void pompeii::Transform::SetLocalScale(const glm::vec3& scale)			{ m_LocalScale = scale; SetScaleDirty(); }

const glm::mat4& pompeii::Transform::GetMatrix()
{
	if (m_DirtyPosition || m_DirtyEulerAngles || m_DirtyScale)
		RecalculateMatrix();
	return m_WorldMatrix;
}


void pompeii::Transform::SetTransformDirty()
{
	SetPositionDirty();
	SetEulerAnglesDirty();
	SetScaleDirty();
}
void pompeii::Transform::SetPositionDirty()
{
	m_DirtyPosition = true;
	for (const auto& child : m_vChildren)
		child->SetPositionDirty();
}
void pompeii::Transform::SetEulerAnglesDirty()
{
	m_DirtyEulerAngles = true;
	for (const auto& child : m_vChildren)
		child->SetEulerAnglesDirty();
}
void pompeii::Transform::SetScaleDirty()
{
	m_DirtyScale = true;
	for (const auto& child : m_vChildren)
		child->SetScaleDirty();
}

void pompeii::Transform::DecomposeMatrixToLocals(const glm::mat4& mat)
{
	glm::vec3 scale;
	glm::quat orientation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;

	if (glm::decompose(mat, scale, orientation, translation, skew, perspective))
	{
		SetLocalPosition(translation);
		SetLocalScale(scale);
		glm::vec3 euler = glm::degrees(glm::eulerAngles(orientation));
		SetLocalEulerAngles(euler);
	}
}
void pompeii::Transform::RecalculateMatrix()
{
	RecalculatePosition();
	RecalculateEulerAngles();
	RecalculateScale();

	const glm::mat4 T = glm::translate(glm::mat4(1.0f), m_Position);
	const glm::quat rotationQuaternion = glm::quat(glm::radians(m_EulerAngles));
	const glm::mat4 R = glm::mat4_cast(rotationQuaternion);
	const glm::mat4 S = glm::scale(glm::mat4(1.0f), m_Scale);

	m_WorldMatrix = T * R * S;
}
void pompeii::Transform::RecalculatePosition()
{
	if (m_pParent)
		m_Position = glm::vec3(m_pParent->GetMatrix() * glm::vec4(m_LocalPosition, 1.0f));
	else m_Position = m_LocalPosition;
	m_DirtyPosition = false;
}
void pompeii::Transform::RecalculateEulerAngles()
{
	if (m_pParent)
		m_EulerAngles = glm::degrees(glm::eulerAngles(glm::quat(glm::radians(m_pParent->GetEulerAngles())) * glm::quat(glm::radians(m_LocalEulerAngles))));
	else m_EulerAngles = m_LocalEulerAngles;
	m_DirtyEulerAngles = false;
}
void pompeii::Transform::RecalculateScale()
{
	if (m_pParent)
		m_Scale = m_pParent->GetScale() * m_LocalScale;
	else m_Scale = m_LocalScale;
	m_DirtyScale = false;
}