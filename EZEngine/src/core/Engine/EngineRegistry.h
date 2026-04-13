#pragma once
#ifndef __CORE_ENGINE_REGISTRY_H__
#define __CORE_ENGINE_REGISTRY_H__

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace EZ
{
	class IEngine;

	class EngineRegistry
	{
	public:
		using EngineFactory = std::function<std::unique_ptr<IEngine>()>;

	public:
		static EngineRegistry& Get()
		{
			static EngineRegistry instance;
			return instance;
		}

		void Register(const std::string& engineId, EngineFactory factory)
		{
			m_Factories[engineId] = std::move(factory);
		}

		std::unique_ptr<IEngine> Create(const std::string& engineId) const
		{
			auto it = m_Factories.find(engineId);
			if (it == m_Factories.end())
			{
				return nullptr;
			}

			return it->second();
		}

		bool Has(const std::string& engineId) const
		{
			return m_Factories.find(engineId) != m_Factories.end();
		}

		std::vector<std::string> GetRegisteredEngineIds() const
		{
			std::vector<std::string> ids;
			ids.reserve(m_Factories.size());
			for (const auto& [id, _] : m_Factories)
			{
				(void)_;
				ids.push_back(id);
			}
			return ids;
		}

	private:
		std::unordered_map<std::string, EngineFactory> m_Factories;
	};
}

#define EZ_ENGINE_CONCAT_IMPL(x, y) x##y
#define EZ_ENGINE_CONCAT(x, y) EZ_ENGINE_CONCAT_IMPL(x, y)

#define EZ_REGISTER_ENGINE(engineClass, engineId)                              \
    namespace                                                                  \
    {                                                                          \
        struct EZ_ENGINE_CONCAT(engineClass, _AutoEngineRegister_)             \
        {                                                                      \
            EZ_ENGINE_CONCAT(engineClass, _AutoEngineRegister_)()              \
            {                                                                  \
                EZ::EngineRegistry::Get().Register((engineId), []()            \
                {                                                              \
                    return std::make_unique<engineClass>();                    \
                });                                                            \
            }                                                                  \
        };                                                                     \
                                                                               \
        static EZ_ENGINE_CONCAT(engineClass, _AutoEngineRegister_)             \
            EZ_ENGINE_CONCAT(s_, engineClass##_AutoEngineRegister_Instance);   \
    }

#define EZ_REGISTER_ENGINE_AUTO_ID(engineClass) \
    EZ_REGISTER_ENGINE(engineClass, #engineClass)

#endif
