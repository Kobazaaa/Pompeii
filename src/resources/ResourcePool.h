#ifndef RESOURCE_POOL_H
#define RESOURCE_POOL_H

// -- Standard Library --
#include <cstdint>
#include <utility>
#include <vector>

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  ResourcePool
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	template <typename T>
	class ResourcePool
	{
	public:
		//--------------------------------------------------
		//    Assert - Can't use 'required' keyword because i don't wanna have to write that at each forward declaration
		//--------------------------------------------------
		static_assert(std::is_default_constructible_v<T>, "T must be default constructible");
		static_assert(std::is_move_assignable_v<T>, "T must be move assignable");

		//--------------------------------------------------
		//    Allocation
		//--------------------------------------------------
		template <typename HandleT>
		HandleT Emplace(T&& resource)
		{
			uint32_t idx{};
			if (!m_FreeList.empty())
			{
				idx = m_FreeList.back();
				m_FreeList.pop_back();
				m_vSlots[idx].resource = std::move(resource);
				m_vSlots[idx].isAlive = true;
			}
			else
			{
				idx = static_cast<uint32_t>(m_vSlots.size());
				m_vSlots.push_back(Slot{ std::move(resource), 0, true });
			}
			return HandleT{ idx, m_vSlots[idx].generation };
		}

		template <typename HandleT>
		bool Destroy(HandleT handle)
		{
			return Destroy(handle, [](T&) {});
		}
		template <typename HandleT, typename Fn>
		bool Destroy(HandleT handle, Fn&& cleanup)
		{
			if (!IsValid(handle)) return false;

			Slot& slot = m_vSlots[handle.index];
			cleanup(slot.resource);
			slot.resource = T{};
			slot.isAlive = false;
			++slot.generation;
			m_FreeList.push_back(handle.index);
			return true;
		}

		//--------------------------------------------------
		//    Access
		//--------------------------------------------------
		template <typename HandleT>
		bool IsValid(HandleT handle) const
		{
			if (!handle.IsValid() || handle.index >= m_vSlots.size()) return false;
			const Slot& slot = m_vSlots[handle.index];
			return slot.isAlive && slot.generation == handle.generation;
		}

		template <typename HandleT>
		T* TryGet(HandleT handle)
		{
			return IsValid(handle) ? &m_vSlots[handle.index].resource : nullptr;
		}
		template <typename HandleT>
		const T* TryGet(HandleT handle) const
		{
			return IsValid(handle) ? &m_vSlots[handle.index].resource : nullptr;
		}

		template <typename Fn>
		void ForEachAlive(Fn&& fn)
		{
			for (Slot& slot : m_vSlots)
				if (slot.isAlive)
					fn(slot.resource);
		}
		template <typename Fn>
		void ForEachAlive(Fn&& fn) const
		{
			for (const Slot& slot : m_vSlots)
				if (slot.isAlive)
					fn(slot.resource);
		}

		uint32_t AliveCount() const { return static_cast<uint32_t>(m_vSlots.size() - m_FreeList.size()); }
		uint32_t Capacity() const { return static_cast<uint32_t>(m_vSlots.size()); }

	private:
		struct Slot
		{
			T resource{};
			uint32_t generation{ 0 };
			bool isAlive{ false };
		};
		std::vector<Slot> m_vSlots{};
		std::vector<uint32_t> m_FreeList{};
	};
}

#endif // RESOURCE_POOL_H
