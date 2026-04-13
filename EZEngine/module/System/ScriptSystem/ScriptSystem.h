#pragma once
#ifndef __SCRIPT_SYSTEM_H__
#define __SCRIPT_SYSTEM_H__

#include "core/System/ISystem.h"

class ScriptSystem : public EZ::ISystem
{
public:
	const char* GetName() const override { return "ScriptSystem"; }

	void Shutdown(EZ::ProjectContext& project, EZ::WorldContext& world) override;
	void Update(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime) override;
	void FixedUpdate(EZ::ProjectContext& project, EZ::WorldContext& world, float fixedDeltaTime) override;
	void LateUpdate(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime) override;
};

#endif