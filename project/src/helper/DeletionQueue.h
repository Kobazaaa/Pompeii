#ifndef DELETION_QUEUE_H
#define DELETION_QUEUE_H

// -- Standard Library --
#include <functional>
#include <vector>

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  DeletionQueue	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class DeletionQueue
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		DeletionQueue() = default;

		//--------------------------------------------------
		//    Queue
		//--------------------------------------------------
		void Push(const std::function<void()>& dtor);
		void Flush();

	private:
		std::vector<std::function<void()>> m_Destructors;
	};
}
#endif // DELETION_QUEUE_H