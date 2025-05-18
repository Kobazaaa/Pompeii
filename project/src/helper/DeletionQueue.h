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
		~DeletionQueue() = default;

		DeletionQueue(const DeletionQueue& other) = delete;
		DeletionQueue(DeletionQueue&& other) noexcept = delete;
		DeletionQueue& operator=(const DeletionQueue& other) = delete;
		DeletionQueue& operator=(DeletionQueue&& other) noexcept = delete;

		//--------------------------------------------------
		//    Queue
		//--------------------------------------------------
		uint32_t Push(const std::function<void()>& dtor);
		void Flush();
		void Erase(uint32_t idx);

	private:
		std::vector<std::function<void()>> m_Destructors;
	};
}
#endif // DELETION_QUEUE_H