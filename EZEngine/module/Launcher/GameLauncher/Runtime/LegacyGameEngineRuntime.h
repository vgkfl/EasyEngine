#pragma once
#ifndef __LEGACY_GAME_ENGINE_RUNTIME_H__
#define __LEGACY_GAME_ENGINE_RUNTIME_H__

#include "core/Engine/IEngineRuntime.h"
#include "Launcher/GameLauncher/RunTimeContext/WorldRunTime.h"

class LegacyGameEngineRuntime final : public EZ::IEngineRuntime
{
public:
	int Initialize(EZ::ProjectContext& project) override
	{
		return m_Runtime.Initialize(project);
	}

	bool Tick(EZ::ProjectContext& project) override
	{
		auto& world = m_Runtime.GetWorldContext();
		if (!world.HasOpenWindow())
		{
			return false;
		}

		world.PumpWindowEvents();
		m_Runtime.Tick(project);
		return world.HasOpenWindow();
	}

	int Shutdown(EZ::ProjectContext& project) override
	{
		(void)project;
		m_Runtime.Shutdown();
		return 0;
	}

	EZ::WorldContext& GetWorldContext() override
	{
		return m_Runtime.GetWorldContext();
	}

	const EZ::WorldContext& GetWorldContext() const override
	{
		return m_Runtime.GetWorldContext();
	}

private:
	WorldRuntime m_Runtime;
};

#endif