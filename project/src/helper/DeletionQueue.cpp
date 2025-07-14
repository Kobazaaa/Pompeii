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
uint32_t pompeii::DeletionQueue::Push(const std::function<void()>& dtor)
{
	m_Destructors.push_back(dtor);
    return static_cast<uint32_t>(m_Destructors.size() - 1);
}
void pompeii::DeletionQueue::Flush()
{
    for (auto& destructor : std::ranges::reverse_view(m_Destructors))
        destructor();
    m_Destructors.clear();
}
void pompeii::DeletionQueue::Erase(uint32_t idx)
{
    uint32_t count = 0;
    auto it = std::ranges::find_if(m_Destructors, [&](const auto&)
    {
		bool ret = false;
        if (count == idx)
            ret = true;
        ++count;
	    return ret;
    });
    if (it != m_Destructors.end())
		m_Destructors.erase(it);
}
