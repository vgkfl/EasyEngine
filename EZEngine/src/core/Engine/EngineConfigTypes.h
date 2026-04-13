#pragma once
#ifndef __CORE_ENGINE_CONFIG_TYPES_H__
#define __CORE_ENGINE_CONFIG_TYPES_H__

#include <string>
#include <utility>
#include <vector>

#include "core/Types.h"

namespace EZ
{
	struct ConfigKV
	{
		static constexpr const char* TypeName() noexcept { return "ConfigKV"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("key", key);
			v("value", value);
		}

		std::string key;
		std::string value;
	};

	struct ConfigTableBase
	{
		const std::string* FindValue(const std::string& key) const
		{
			for (const auto& entry : entries)
			{
				if (entry.key == key)
				{
					return &entry.value;
				}
			}
			return nullptr;
		}

		std::string* FindValue(const std::string& key)
		{
			for (auto& entry : entries)
			{
				if (entry.key == key)
				{
					return &entry.value;
				}
			}
			return nullptr;
		}

		bool Contains(const std::string& key) const
		{
			return FindValue(key) != nullptr;
		}

		void SetValue(std::string key, std::string value)
		{
			if (std::string* current = FindValue(key))
			{
				*current = std::move(value);
				return;
			}

			entries.push_back(ConfigKV{ std::move(key), std::move(value) });
		}

		bool RemoveValue(const std::string& key)
		{
			for (auto it = entries.begin(); it != entries.end(); ++it)
			{
				if (it->key == key)
				{
					entries.erase(it);
					return true;
				}
			}
			return false;
		}

		void Clear()
		{
			entries.clear();
		}

		std::vector<ConfigKV> entries;
	};

	struct EngineConfigTable : public ConfigTableBase
	{
		static constexpr const char* TypeName() noexcept { return "EngineConfigTable"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("entries", entries);
		}
	};

	struct ProjectConfigTable : public ConfigTableBase
	{
		static constexpr const char* TypeName() noexcept { return "ProjectConfigTable"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("entries", entries);
		}
	};

	struct ProjectLaunchProfile
	{
		static constexpr const char* TypeName() noexcept { return "ProjectLaunchProfile"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("engineId", engineId);
			v("engineEntryKey", engineEntryKey);
			v("startupScene", startupScene);
			v("engineConfig", engineConfig);
			v("projectConfig", projectConfig);
		}

		std::string engineId;
		std::string engineEntryKey;
		std::string startupScene;
		EngineConfigTable engineConfig;
		ProjectConfigTable projectConfig;
	};
}

#endif