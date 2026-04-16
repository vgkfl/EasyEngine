#include "Launcher/GameLauncher/GameTemplateEngine.h"

#include "DataProtocol/WindowData.h"
#include "core/Engine/EngineRegistry.h"

void GameTemplateEngine::SetupMainWindowDesc(DataProtocol::WindowDesc& desc) const
{
	desc.title = "EZEngine";
	desc.size = { 1600, 900 };
	desc.backendHint = DataProtocol::WindowBackendHint::OpenGL;
}

EZ_REGISTER_ENGINE(GameTemplateEngine, "ez.game_launcher.engine");