#pragma once
#ifndef __BASE_PROTOCOL_SCRIPT_COMPONENT_H__
#define __BASE_PROTOCOL_SCRIPT_COMPONENT_H__

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "core/ScriptManager/IScriptBehaviour.h"

namespace BaseProtocol
{
	struct ScriptEntry
	{
		std::string scriptName;

		bool enabled = true;
		bool wasEnabled = false;
		bool awoken = false;
		bool started = false;
		bool destroyRequested = false;

		std::unique_ptr<EZ::IScriptBehaviour> instance;

		ScriptEntry() = default;

		ScriptEntry(const ScriptEntry& other)
			: scriptName(other.scriptName)
			, enabled(other.enabled)
			, wasEnabled(false)
			, awoken(false)
			, started(false)
			, destroyRequested(other.destroyRequested)
			, instance(nullptr)
		{
		}

		ScriptEntry& operator=(const ScriptEntry& other)
		{
			if (this == &other)
			{
				return *this;
			}

			scriptName = other.scriptName;
			enabled = other.enabled;
			wasEnabled = false;
			awoken = false;
			started = false;
			destroyRequested = other.destroyRequested;
			instance.reset();
			return *this;
		}

		ScriptEntry(ScriptEntry&&) noexcept = default;
		ScriptEntry& operator=(ScriptEntry&&) noexcept = default;
	};

	struct ScriptComponent
	{
		using ScriptList = std::vector<ScriptEntry>;

		ScriptEntry& AddScript(std::string scriptName)
		{
			scripts.emplace_back();
			scripts.back().scriptName = std::move(scriptName);
			return scripts.back();
		}

		ScriptEntry* FindScript(const std::string& scriptName)
		{
			for (auto& script : scripts)
			{
				if (script.scriptName == scriptName)
				{
					return &script;
				}
			}
			return nullptr;
		}

		const ScriptEntry* FindScript(const std::string& scriptName) const
		{
			for (const auto& script : scripts)
			{
				if (script.scriptName == scriptName)
				{
					return &script;
				}
			}
			return nullptr;
		}

		bool HasScript(const std::string& scriptName) const
		{
			return FindScript(scriptName) != nullptr;
		}

		bool RequestDestroy(const std::string& scriptName)
		{
			auto* script = FindScript(scriptName);
			if (!script)
			{
				return false;
			}

			script->destroyRequested = true;
			return true;
		}

		ScriptList scripts;
	};
}

#endif