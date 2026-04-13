#include "GameLauncher.h"

#include <chrono>
#include <memory>

#include "DataProtocol/WindowData.h"
#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Engine/EngineRegistry.h"
#include "core/ProjectManager/IProject.h"
#include "Launcher/GameLauncher/Runtime/LegacyGameEngineRuntime.h"

#include "../../../Project/Test01/Test01.h"

namespace
{
	std::unique_ptr<EZ::IProject> CreateProject()
	{
		return std::make_unique<Test01Project>();
	}
}

int GameLauncher::Start()
{
	int ret = Init();
	if (ret != 0)
	{
		return ret;
	}

	EZ::ProjectContext projectCtx;
	projectCtx.engine = this;

	std::unique_ptr<EZ::IProject> project = CreateProject();
	projectCtx.project = project.get();

	if (projectCtx.engine)
	{
		ret = projectCtx.engine->OnAttachProjectContext(projectCtx);
		if (ret != 0)
		{
			return ret;
		}
	}

	if (project)
	{
		ret = project->OnPreEngineInit(projectCtx);
		if (ret != 0) return ret;

		ret = project->OnEngineInit(projectCtx);
		if (ret != 0) return ret;

		ret = project->OnPostEngineInit(projectCtx);
		if (ret != 0) return ret;
	}

	auto runtime = projectCtx.engine ? projectCtx.engine->CreateRuntime(projectCtx) : nullptr;
	if (!runtime)
	{
		runtime = std::make_unique<LegacyGameEngineRuntime>();
	}

	ret = runtime->Initialize(projectCtx);
	if (ret != 0)
	{
		if (projectCtx.engine)
		{
			projectCtx.engine->OnDetachProjectContext(projectCtx);
		}
		return ret;
	}

	if (project)
	{
		ret = project->OnBeforeMainLoop(projectCtx);
		if (ret != 0)
		{
			runtime->Shutdown(projectCtx);
			if (projectCtx.engine)
			{
				projectCtx.engine->OnDetachProjectContext(projectCtx);
			}
			return ret;
		}
	}

	auto lastTickTime = std::chrono::steady_clock::now();
	while (runtime->GetWorldContext().HasOpenWindow() && Tick())
	{
		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<float> delta = now - lastTickTime;
		lastTickTime = now;

		projectCtx.deltaTime = delta.count();
		if (projectCtx.deltaTime < 0.0f)
		{
			projectCtx.deltaTime = 0.0f;
		}
		else if (projectCtx.deltaTime > 0.25f)
		{
			projectCtx.deltaTime = 0.25f;
		}

		if (!runtime->Tick(projectCtx))
		{
			break;
		}
	}

	if (project)
	{
		project->OnAfterMainLoop(projectCtx);
	}

	runtime->Shutdown(projectCtx);

	if (project)
	{
		project->OnBeforeEngineShutdown(projectCtx);
		project->OnEngineShutdown(projectCtx);
	}

	if (projectCtx.engine)
	{
		projectCtx.engine->OnDetachProjectContext(projectCtx);
	}

	return Destroy();
}

void GameLauncher::SetupMainWindowDesc(DataProtocol::WindowDesc& desc) const
{
	desc.title = "EZEngine";
	desc.size = { 1600, 900 };
	desc.backendHint = DataProtocol::WindowBackendHint::OpenGL;
}

std::unique_ptr<EZ::IEngineRuntime> GameLauncher::CreateRuntime(EZ::ProjectContext& ctx)
{
	(void)ctx;
	return std::make_unique<LegacyGameEngineRuntime>();
}

int GameLauncher::Init()
{
	return 0;
}

bool GameLauncher::Tick() const
{
	return true;
}

int GameLauncher::Destroy()
{
	return 0;
}

EZ_REGISTER_LAUNCHER(GameLauncher, "GameLauncher");
EZ_REGISTER_ENGINE(GameLauncher, "ez.game_launcher.engine");