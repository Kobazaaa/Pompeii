#ifndef POMPEII_DEBUG_MESSENGER_INTERFACE_H
#define POMPEII_DEBUG_MESSENGER_INTERFACE_H

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    IDebugMessenger
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class IDebugMessenger
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		virtual ~IDebugMessenger() = default;

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
	};
}

#endif // POMPEII_DEBUG_MESSENGER_INTERFACE_H
