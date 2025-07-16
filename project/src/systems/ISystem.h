#ifndef INTERFACE_SYSTEM_H
#define INTERFACE_SYSTEM_H

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  ISystem	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class ISystem
	{
	public:
		virtual void BeginFrame() {}
		virtual void Update() {}
		virtual void EndFrame() {}

		virtual ~ISystem() = default;
	};
}

#endif // INTERFACE_SYSTEM_H
