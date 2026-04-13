#include <cstdio>

#include "core/LauncherManager/LauncherManager.h"

int main()
{
	std::printf("[main] program start\n");

	auto launcher = EZ::LauncherManager::Get().Create("GameLauncher");
	if (!launcher)
	{
		std::printf("[main] Create(GameLauncher) failed\n");
		std::fflush(stdout);
		return -1;
	}

	std::printf("[main] launcher created\n");
	std::fflush(stdout);

	const int ret = launcher->Start();

	std::printf("[main] launcher finished, ret=%d\n", ret);
	std::fflush(stdout);

	return ret;
}