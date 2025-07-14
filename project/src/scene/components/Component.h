#ifndef COMPONENT_H
#define COMPONENT_H

// -- Forward Declarations --
namespace pompeii
{
	class SceneObject;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Component	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Component
	{
	public:
		//--------------------------------------------------
		//    Constructor and Destructor
		//--------------------------------------------------
		explicit Component(SceneObject& parent) : m_pParent{ &parent } {}
		virtual ~Component() = default;
		Component(const Component& other) = delete;
		Component(Component&& other) = delete;
		Component& operator=(const Component& other) = delete;
		Component& operator=(Component&& other) = delete;


		//--------------------------------------------------
		//    Loop
		//--------------------------------------------------
		virtual void Start() {}
		virtual void Update() {}
		virtual void OnImGuiRender() {}

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		void Destroy()											{ m_DeletionFlag = true; }
		[[nodiscard]] bool IsFlaggedForDestruction()	const	{ return m_pParent; }
		[[nodiscard]] SceneObject* GetSceneObject()		const	{ return m_pParent; }
		bool isActive = true;

	private:
		SceneObject* m_pParent	{ nullptr };
		bool m_DeletionFlag		{ false };
	};
}

#endif // COMPONENT_H
