#ifndef DELETION_QUEUE_H
#define DELETION_QUEUE_H

#include <functional>
#include <vector>

namespace pom
{
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
		void Push(std::function<void()> dtor);
		void Flush();

	private:
		std::vector<std::function<void()>> m_Destructors;
	};
}
#endif // DELETION_QUEUE_H