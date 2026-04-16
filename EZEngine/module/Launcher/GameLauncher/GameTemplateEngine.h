#pragma once
#ifndef __GAME_TEMPLATE_ENGINE_H__
#define __GAME_TEMPLATE_ENGINE_H__

#include "TemplateEngineBase/TemplateEngineBase.h"

namespace DataProtocol
{
	struct WindowDesc;
}

class GameTemplateEngine final : public EZ::TemplateEngineBase
{
public:
	const char* GetEngineId() const override { return "ez.game_launcher.engine"; }
	const char* GetEngineDisplayName() const override { return "Game Template Engine"; }

	void SetupMainWindowDesc(DataProtocol::WindowDesc& desc) const override;

protected:
	const char* GetDefaultLauncherId() const override { return "GameLauncher"; }
};

#endif