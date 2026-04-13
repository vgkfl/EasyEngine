#pragma once
#ifndef __INPUT_SYSTEM_H__
#define __INPUT_SYSTEM_H__

#include "core/System/ISystem.h"

class InputSystem : public EZ::ISystem
{
public:
	const char* GetName() const override { return "InputSystem"; }

	void Update(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime) override;
};

#endif
