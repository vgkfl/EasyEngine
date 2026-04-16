#include <cstdio>
#include <memory>

#include "core/Engine/EngineRegistry.h"
#include "core/Engine/IEngine.h"

#include "../Project/Test01/Test01.h"

int main()
{
	std::printf("[main] program start\n");

	auto engine = EZ::EngineRegistry::Get().Create("ez.game_launcher.engine");
	if (!engine)
	{
		std::printf("[main] create engine failed\n");
		std::fflush(stdout);
		return -1;
	}

	std::printf("[main] engine created\n");
	std::fflush(stdout);

	std::unique_ptr<EZ::IProject> project = std::make_unique<Test01Project>();

	const int ret = engine->Start(project.get());

	std::printf("[main] engine finished, ret=%d\n", ret);
	std::fflush(stdout);

	return ret;
}