#include "Launcher/GameLauncher/GameLauncher.h"

#include <chrono>

#include "core/LauncherManager/LauncherManager.h"

int GameLauncher::Start(EZ::IProgramRunner& runner)
{
	m_ShouldQuit = false;

	int ret = runner.Initialize();
	if (ret != 0)
	{
		return ret;
	}

	auto lastTickTime = std::chrono::steady_clock::now();

	while (!ShouldQuit())
	{
		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<float> delta = now - lastTickTime;
		lastTickTime = now;

		float deltaTime = delta.count();
		if (deltaTime < 0.0f)
		{
			deltaTime = 0.0f;
		}
		else if (deltaTime > 0.25f)
		{
			deltaTime = 0.25f;
		}

		if (!runner.TickFrame(deltaTime))
		{
			RequestQuit();
			break;
		}
	}

	return runner.Shutdown();
}

void GameLauncher::RequestQuit()
{
	m_ShouldQuit = true;
}

bool GameLauncher::ShouldQuit() const
{
	return m_ShouldQuit;
}

EZ_REGISTER_LAUNCHER(GameLauncher, "GameLauncher");