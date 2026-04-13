#pragma once
#ifndef __CORE_SCRIPT_REGISTRY_H__
#define __CORE_SCRIPT_REGISTRY_H__

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "core/ScriptManager/IScriptBehaviour.h"

namespace EZ
{
	class ScriptRegistry
	{
	public:
		using Factory = std::function<std::unique_ptr<IScriptBehaviour>()>;

		bool RegisterFactory(const std::string& name, Factory factory, bool allowOverride = false)
		{
			if (name.empty() || !factory)
			{
				return false;
			}

			auto it = m_Factories.find(name);
			if (it != m_Factories.end() && !allowOverride)
			{
				return false;
			}

			m_Factories[name] = std::move(factory);
			return true;
		}

		template<typename T>
		bool Register(const std::string& name, bool allowOverride = false)
		{
			static_assert(std::is_base_of_v<IScriptBehaviour, T>, "T must derive from IScriptBehaviour");

			return RegisterFactory(name, []()
				{
					return std::make_unique<T>();
				}, allowOverride);
		}

		std::unique_ptr<IScriptBehaviour> Create(const std::string& name) const
		{
			auto it = m_Factories.find(name);
			if (it == m_Factories.end())
			{
				return nullptr;
			}

			return it->second();
		}

		bool Has(const std::string& name) const
		{
			return m_Factories.find(name) != m_Factories.end();
		}

		bool Unregister(const std::string& name)
		{
			return m_Factories.erase(name) > 0;
		}

		void Clear()
		{
			m_Factories.clear();
		}

	private:
		std::unordered_map<std::string, Factory> m_Factories;
	};
}

#endif