#include "ScriptSystem.h"

#include <cstddef>

#include "BaseProtocol/Script/ScriptComponent.h"
#include "ControlProtocol/EntityManager/EntityManager.h"
#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Context/RunTimeContext/WorldContext.h"
#include "core/ScriptManager/ScriptRegistry.h"

namespace
{
	using ScriptEntry = BaseProtocol::ScriptEntry;

	bool EnsureScriptInstance(
		ScriptEntry& script,
		EZ::Entity entity,
		EZ::WorldContext& world,
		EZ::ScriptRegistry* registry)
	{
		if (script.instance)
		{
			return true;
		}

		if (!registry || script.scriptName.empty())
		{
			return false;
		}

		auto instance = registry->Create(script.scriptName);
		if (!instance)
		{
			return false;
		}

		instance->Bind(&world, entity);
		script.instance = std::move(instance);
		return true;
	}

	void RefreshEnableState(ScriptEntry& script)
	{
		if (!script.instance)
		{
			return;
		}

		if (script.enabled && !script.wasEnabled)
		{
			script.instance->OnEnable();
			script.wasEnabled = true;
		}
		else if (!script.enabled && script.wasEnabled)
		{
			script.instance->OnDisable();
			script.wasEnabled = false;
		}
	}

	void DestroyScript(ScriptEntry& script)
	{
		if (script.instance)
		{
			if (script.wasEnabled)
			{
				script.instance->OnDisable();
				script.wasEnabled = false;
			}

			script.instance->OnDestroy();
			script.instance.reset();
		}

		script.awoken = false;
		script.started = false;
		script.destroyRequested = false;
	}
}

void ScriptSystem::Shutdown(EZ::ProjectContext& project, EZ::WorldContext& world)
{
	(void)project;

	auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
	if (!entityManager)
	{
		return;
	}

	entityManager->ForEach<BaseProtocol::ScriptComponent>(
		[&](EZ::Entity entity, BaseProtocol::ScriptComponent& scriptComponent)
		{
			(void)entity;
			for (auto& script : scriptComponent.scripts)
			{
				DestroyScript(script);
			}
			scriptComponent.scripts.clear();
		});
}

void ScriptSystem::Update(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime)
{
	(void)project;
	(void)deltaTime;

	auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
	auto* registry = world.TryGet<EZ::ScriptRegistry>();

	if (!entityManager)
	{
		return;
	}

	entityManager->ForEach<BaseProtocol::ScriptComponent>(
		[&](EZ::Entity entity, BaseProtocol::ScriptComponent& scriptComponent)
		{
			for (std::size_t i = 0; i < scriptComponent.scripts.size();)
			{
				auto& script = scriptComponent.scripts[i];
				bool eraseCurrent = false;

				if (script.destroyRequested)
				{
					DestroyScript(script);
					eraseCurrent = true;
				}
				else if (EnsureScriptInstance(script, entity, world, registry))
				{
					if (!script.awoken)
					{
						script.instance->Awake();
						script.awoken = true;
					}

					RefreshEnableState(script);

					if (script.enabled)
					{
						if (!script.started)
						{
							script.instance->Start();
							script.started = true;
						}

						script.instance->Update();
					}

					if (script.destroyRequested)
					{
						DestroyScript(script);
						eraseCurrent = true;
					}
				}

				if (eraseCurrent)
				{
					scriptComponent.scripts.erase(
						scriptComponent.scripts.begin() + static_cast<std::ptrdiff_t>(i));
				}
				else
				{
					++i;
				}
			}
		});
}

void ScriptSystem::FixedUpdate(EZ::ProjectContext& project, EZ::WorldContext& world, float fixedDeltaTime)
{
	(void)project;
	(void)fixedDeltaTime;

	auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
	if (!entityManager)
	{
		return;
	}

	entityManager->ForEach<BaseProtocol::ScriptComponent>(
		[&](EZ::Entity entity, BaseProtocol::ScriptComponent& scriptComponent)
		{
			(void)entity;
			for (auto& script : scriptComponent.scripts)
			{
				if (!script.instance || !script.enabled || !script.started ||
					script.destroyRequested || !script.wasEnabled)
				{
					continue;
				}

				script.instance->FixedUpdate();
			}
		});
}

void ScriptSystem::LateUpdate(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime)
{
	(void)project;
	(void)deltaTime;

	auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
	if (!entityManager)
	{
		return;
	}

	entityManager->ForEach<BaseProtocol::ScriptComponent>(
		[&](EZ::Entity entity, BaseProtocol::ScriptComponent& scriptComponent)
		{
			(void)entity;
			for (auto& script : scriptComponent.scripts)
			{
				if (!script.instance || !script.enabled || !script.started ||
					script.destroyRequested || !script.wasEnabled)
				{
					continue;
				}

				script.instance->LateUpdate();
			}
		});
}