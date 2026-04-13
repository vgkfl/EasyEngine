#include "PhysicsSystem.h"

#include "ControlProtocol/PhysicsWorldController/PhysicsWorldController.h"
#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Context/RunTimeContext/WorldContext.h"

int PhysicsSystem::Initialize(EZ::ProjectContext& project, EZ::WorldContext& world)
{
	(void)project;

	auto* physicsWorld = world.TryGet<ControlProtocol::PhysicsWorldController>();
	if (!physicsWorld)
	{
		return 0;
	}

	return physicsWorld->Initialize() ? 0 : -1;
}

void PhysicsSystem::Shutdown(EZ::ProjectContext& project, EZ::WorldContext& world)
{
	(void)project;

	auto* physicsWorld = world.TryGet<ControlProtocol::PhysicsWorldController>();
	if (!physicsWorld)
	{
		return;
	}

	physicsWorld->Shutdown();
}

void PhysicsSystem::FixedUpdate(EZ::ProjectContext& project, EZ::WorldContext& world, float fixedDeltaTime)
{
	(void)project;

	auto* physicsWorld = world.TryGet<ControlProtocol::PhysicsWorldController>();
	if (!physicsWorld)
	{
		return;
	}

	physicsWorld->StepSimulation(fixedDeltaTime);
}