// -- Standard Library --
#include <ranges>

// -- Pompeii Includes --
#include "DeletionQueue.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  DeletionQueue	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Builder
//--------------------------------------------------
void pom::DeletionQueue::Push(const std::function<void()>& dtor)
{
	m_Destructors.push_back(dtor);
}
void pom::DeletionQueue::Flush()
{
    for (auto& destructor : std::ranges::reverse_view(m_Destructors))
        destructor();
    m_Destructors.clear();
}
