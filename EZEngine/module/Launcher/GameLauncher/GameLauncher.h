#pragma once
#ifndef __GAME_LAUNCHER_H__
#define __GAME_LAUNCHER_H__

#include "core/LauncherManager/ILauncher.h"

class GameLauncher final : public EZ::ILauncher
{
public:
	int Start(EZ::IProgramRunner& runner) override;
	void RequestQuit() override;
	bool ShouldQuit() const override;

private:
	bool m_ShouldQuit = false;
};

#endif