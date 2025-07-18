// -- Pompeii Includes --
#include "Transform.h"

//--------------------------------------------------
//    Constructor
//--------------------------------------------------
pompeii::Transform::Transform(SceneObject* pOwner)
	: m_pSceneObject{ pOwner }
{}

//--------------------------------------------------
//    Parent-Child
//--------------------------------------------------
pompeii::SceneObject* pompeii::Transform::GetSceneObject() const { return m_pSceneObject; }
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
// -- World --
const glm::vec3& pompeii::Transform::GetPosition()						{ if (m_DirtyPosition) RecalculatePosition(); return m_Position; }
const glm::vec3& pompeii::Transform::GetEulerAngles()					{ if (m_DirtyEulerAngles) RecalculateEulerAngles(); return m_EulerAngles; }
const glm::vec3& pompeii::Transform::GetScale()							{ if (m_DirtyScale) RecalculateScale(); return m_Scale; }

const glm::vec3& pompeii::Transform::GetForward()						{ if (m_DirtyEulerAngles || m_DirtyScale) RecalculateMatrix(); return m_Forward; }
const glm::vec3& pompeii::Transform::GetRight()							{ if (m_DirtyEulerAngles || m_DirtyScale) RecalculateMatrix(); return m_Right; }
const glm::vec3& pompeii::Transform::GetUp()							{ if (m_DirtyEulerAngles || m_DirtyScale) RecalculateMatrix(); return m_Up; }

void pompeii::Transform::SetPosition(const glm::vec3& pos)
{
	if (m_pParent)
	{
		const glm::mat4& parentMatrix = m_pParent->GetMatrix();
		glm::mat4 invParent = glm::inverse(parentMatrix);
		glm::vec4 localPos4 = invParent * glm::vec4(pos, 1.0f);
		m_LocalPosition = glm::vec3(localPos4);
	}
	else
	{
		m_LocalPosition = pos;
	}
	SetPositionDirty();
}
void pompeii::Transform::SetEulerAngles(const glm::vec3& euler)
{
	if (m_pParent)
	{
		const glm::quat rotationQuaternion = glm::quat(glm::radians(euler));
		const glm::mat4 rotationMatrix = glm::mat4_cast(rotationQuaternion);

		const glm::mat4 invParent = glm::inverse(m_pParent->GetMatrix());
		const glm::mat4 localRot = invParent * rotationMatrix;
		m_LocalEulerAngles = glm::degrees(glm::eulerAngles(glm::quat_cast(localRot)));
	}
	else
	{
		m_LocalEulerAngles = euler;
	}
	SetEulerAnglesDirty();
}
void pompeii::Transform::SetScale(const glm::vec3& scale)
{
	if (m_pParent)
	{
		const glm::vec3 parentScale = m_pParent->GetScale();
		if (abs(parentScale.x) < FLT_EPSILON || abs(parentScale.y) < FLT_EPSILON || abs(parentScale.z) < FLT_EPSILON)
			m_LocalScale = scale;
		else
			m_LocalScale = scale / parentScale;
	}
	else
	{
		m_LocalScale = scale;
	}
	SetScaleDirty();
}

void pompeii::Transform::Translate(const glm::vec3& translation)		{ SetPosition(GetPosition() + translation); }
void pompeii::Transform::Rotate(const glm::vec3& rotation)				{ SetEulerAngles(GetEulerAngles() + rotation); }
void pompeii::Transform::Scale(const glm::vec3& scale)					{ SetScale(GetScale() + scale); }

const glm::mat4& pompeii::Transform::GetMatrix()
{
	if (m_DirtyPosition || m_DirtyEulerAngles || m_DirtyScale)
		RecalculateMatrix();
	return m_WorldMatrix;
}

// -- Local --
const glm::vec3& pompeii::Transform::GetLocalPosition()		const		{ return m_LocalPosition; }
const glm::vec3& pompeii::Transform::GetLocalEulerAngles()	const		{ return m_LocalEulerAngles; }
const glm::vec3& pompeii::Transform::GetLocalScale()		const		{ return m_LocalScale; }

void pompeii::Transform::SetLocalPosition(const glm::vec3& pos)			{ m_LocalPosition = pos; SetPositionDirty(); }
void pompeii::Transform::SetLocalEulerAngles(const glm::vec3& euler)	{ m_LocalEulerAngles = euler; SetEulerAnglesDirty(); }
void pompeii::Transform::SetLocalScale(const glm::vec3& scale)			{ m_LocalScale = scale; SetScaleDirty(); }

void pompeii::Transform::TranslateLocal(const glm::vec3& translation)	{ SetLocalPosition(m_LocalPosition + translation); }
void pompeii::Transform::RotateLocal(const glm::vec3& rotation)			{ SetLocalEulerAngles(m_LocalEulerAngles + rotation); }
void pompeii::Transform::ScaleLocal(const glm::vec3& scale)				{ SetLocalScale(m_LocalScale + scale); }


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
	m_Right			= glm::normalize(glm::vec3(m_WorldMatrix[0]));
	m_Up			= glm::normalize(glm::vec3(m_WorldMatrix[1]));
	m_Forward		= glm::normalize(glm::vec3(m_WorldMatrix[2]));
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