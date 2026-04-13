#pragma once

#ifndef __GAME_LAUNCHER_H__
#define __GAME_LAUNCHER_H__

#include <memory>

#include "core/Engine/IEngine.h"
#include "core/LauncherManager/ILauncher.h"
#include "core/LauncherManager/LauncherManager.h"

class GameLauncher : public EZ::ILauncher, public EZ::IEngine
{
public:
	int Start() override;

	const char* GetEngineId() const override { return "ez.game_launcher.engine"; }
	const char* GetEngineDisplayName() const override { return "Game Launcher Engine"; }
	void SetupMainWindowDesc(DataProtocol::WindowDesc& desc) const override;
	std::unique_ptr<EZ::IEngineRuntime> CreateRuntime(EZ::ProjectContext& ctx) override;

private:
	int Init();
	bool Tick() const;
	int Destroy();
};
#endif
