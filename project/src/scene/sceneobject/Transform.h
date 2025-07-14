#ifndef TRANSFORM_H
#define TRANSFORM_H

// -- Math Includes --
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

// -- Standard Library --
#include <vector>

// -- Forward Declarations --
namespace pompeii
{
	class SceneObject;
}

namespace pompeii
{
	class Transform final
	{
	public:
		//--------------------------------------------------
		//    Constructor
		//--------------------------------------------------
		explicit Transform(SceneObject* pOwner);

		//--------------------------------------------------
		//    Parent-Child
		//--------------------------------------------------
		SceneObject* GetOwner() const;
		Transform* GetParent() const;
		void SetParent(Transform* parent, bool keepWorldPosition = false);

		bool IsChild(const Transform* child) const;
		int GetChildCount() const;
		const std::vector<Transform*>& GetAllChildren() const;

		//--------------------------------------------------
		//    Transformation
		//--------------------------------------------------
		[[nodiscard]] const glm::vec3& GetPosition();
		[[nodiscard]] const glm::vec3& GetLocalPosition()		const;
		void SetLocalPosition(const glm::vec3& pos);

		[[nodiscard]] const glm::vec3& GetEulerAngles();
		[[nodiscard]] const glm::vec3& GetLocalEulerAngles()	const;
		void SetLocalEulerAngles(const glm::vec3& euler);

		[[nodiscard]] const glm::vec3& GetScale();
		[[nodiscard]] const glm::vec3& GetLocalScale()			const;
		void SetLocalScale(const glm::vec3& scale);

		[[nodiscard]] const glm::mat4& GetMatrix();


	private:
		// -- Parent-Child --
		void AddChild(Transform* child);
		void RemoveChild(const Transform* child);
		Transform* m_pParent{};
		SceneObject* m_pOwnerObject{};
		std::vector<Transform*> m_vChildren{};

		// -- Transformations --
		glm::vec3 m_LocalPosition		{ 0.0f, 0.0f, 0.0f };
		glm::vec3 m_LocalEulerAngles	{ 0.0f, 0.0f, 0.0f };
		glm::vec3 m_LocalScale			{ 1.0f, 1.0f, 1.0f };

		glm::vec3 m_Position			{ 0.0f, 0.0f, 0.0f };
		glm::vec3 m_EulerAngles			{ 0.0f, 0.0f, 0.0f };
		glm::vec3 m_Scale				{ 1.0f, 1.0f, 1.0f };
		glm::mat4 m_WorldMatrix			{ 1.0f };

		bool m_DirtyPosition			{ true };
		bool m_DirtyEulerAngles			{ true };
		bool m_DirtyScale				{ true };

		void SetTransformDirty();
		void SetPositionDirty();
		void SetEulerAnglesDirty();
		void SetScaleDirty();

		void DecomposeMatrixToLocals(const glm::mat4& mat);

		void RecalculateMatrix();
		void RecalculatePosition();
		void RecalculateEulerAngles();
		void RecalculateScale();
	};
}

#endif // TRANSFORM_H
