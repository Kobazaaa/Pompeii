#include "DeletionQueue.h"

#include <ranges>


//--------------------------------------------------
//    Builder
//--------------------------------------------------
void pom::DeletionQueue::Push(std::function<void()> dtor)
{
	m_Destructors.push_back(dtor);
}
void pom::DeletionQueue::Flush()
{
    for (auto& destructor : std::ranges::reverse_view(m_Destructors))
        destructor();
    m_Destructors.clear();
}
