#include "System/TransformSystem/TransformSystem.h"

TransformSystem::TransformSystem(ControlProtocol::EntityManager& entityManager)
	: m_EntityManager(entityManager)
	, m_HierarchySystem(entityManager)
	, m_DirtySystem(entityManager)
	, m_ComputeSystem(entityManager)
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

	m_HierarchySystem.Flush();
	m_DirtySystem.CollectLocalDirty();
	m_DirtySystem.PropagateWorldDirty();
	m_ComputeSystem.UpdateWorldTransforms();
	m_DirtySystem.ClearFrameDirty();
}