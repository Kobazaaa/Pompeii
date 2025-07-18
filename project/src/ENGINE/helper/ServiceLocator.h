#ifndef SERVICE_LOCATOR_H
#define SERVICE_LOCATOR_H

// -- Standard Library --
#include <memory>
#include <typeindex>
#include <stdexcept>

// -- Pompeii Includes --
#include "SceneManager.h"
#include "LightingSystem.h"
#include "RenderSystem.h"

namespace pompeii
{
	class ServiceLocator final
	{
	public:
		ServiceLocator() = delete;
		~ServiceLocator() = delete;

		ServiceLocator(const ServiceLocator&) = delete;
		ServiceLocator(ServiceLocator&&) = delete;
		ServiceLocator& operator=(const ServiceLocator&) = delete;
		ServiceLocator& operator=(ServiceLocator&&) = delete;

		//--------------------------------------------------
		//    Services
		//--------------------------------------------------
		template<typename ServiceType>
		static ServiceType& Get()
		{
			const auto it = m_Services.find(std::type_index(typeid(ServiceType)));
			if (it == m_Services.end())
				throw std::runtime_error("ServiceLocator: Requested service not registered!");

			auto* base = it->second.get();
			auto* derived = dynamic_cast<Service<ServiceType>*>(base);
			if (!derived)
				throw std::runtime_error("ServiceLocator: Type mismatch in service access");

			return *(derived->pService);
		}
		template<typename ServiceType>
		static bool TryGet(ServiceType* pService)
		{
			const auto it = m_Services.find(std::type_index(typeid(ServiceType)));
			if (it == m_Services.end())
			{
				pService = nullptr;
				return false;
			}

			auto* base = it->second.get();
			auto* derived = dynamic_cast<Service<ServiceType>*>(base);
			if (!derived)
			{
				pService = nullptr;
				return false;
			}

			pService = derived->pService;
			return true;
		}

		template<typename ServiceType>
		static void Register(std::unique_ptr<ServiceType> service) { m_Services[std::type_index(typeid(ServiceType))] = std::make_unique<Service<ServiceType>>(std::move(service)); }
		template<typename ServiceType>
		static void Deregister() { m_Services.erase(std::type_index(typeid(ServiceType))); }

	private:
		struct IService { virtual ~IService() = default; };
		template<typename ServiceType>
		struct Service final : IService
		{
			explicit Service(std::unique_ptr<ServiceType> service)
				: pService(std::move(service)) {
			}
			std::unique_ptr<ServiceType> pService;
		};
		inline static std::unordered_map<std::type_index, std::unique_ptr<IService>> m_Services;
	};
}

#endif // SERVICE_LOCATOR_H
