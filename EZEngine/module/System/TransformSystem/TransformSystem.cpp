#include "TransformSystem.h"

#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Context/RunTimeContext/WorldContext.h"

TransformSystem::TransformSystem(ControlProtocol::TransformManager& manager)
	: m_Manager(manager)
{
}

int TransformSystem::Initialize(EZ::ProjectContext& project, EZ::WorldContext& world)
{
	(void)project;
	(void)world;
	return 0;
}

void TransformSystem::LateUpdate(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime)
{
	(void)project;
	(void)world;
	(void)deltaTime;
	m_Manager.Update();
}
