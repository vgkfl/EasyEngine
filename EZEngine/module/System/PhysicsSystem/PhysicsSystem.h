#pragma once
#ifndef __PHYSICS_SYSTEM_H__
#define __PHYSICS_SYSTEM_H__

#include "core/System/ISystem.h"

class PhysicsSystem : public EZ::ISystem
{
public:
	const char* GetName() const override { return "PhysicsSystem"; }

	int Initialize(EZ::ProjectContext& project, EZ::WorldContext& world) override;
	void Shutdown(EZ::ProjectContext& project, EZ::WorldContext& world) override;
	void FixedUpdate(EZ::ProjectContext& project, EZ::WorldContext& world, float fixedDeltaTime) override;
};

#endif