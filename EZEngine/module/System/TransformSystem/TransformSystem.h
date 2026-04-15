#pragma once
#ifndef __TRANSFORM_SYSTEM_H__
#define __TRANSFORM_SYSTEM_H__

#include "core/System/ISystem.h"
#include "ControlProtocol/EntityManager/EntityManager.h"
#include "SubSystems/TransformHierarchySystem.h"
#include "SubSystems/TransformDirtySystem.h"
#include "SubSystems/TransformComputeSystem.h"

class TransformSystem final : public EZ::ISystem
{
public:
	explicit TransformSystem(ControlProtocol::EntityManager& entityManager);

public:
	const char* GetName() const override { return "TransformSystem"; }

	int Initialize(EZ::ProjectContext& project, EZ::WorldContext& world) override;
	void LateUpdate(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime) override;

public:
	TransformHierarchySystem& GetHierarchySystem() { return m_HierarchySystem; }
	TransformDirtySystem& GetDirtySystem() { return m_DirtySystem; }
	TransformComputeSystem& GetComputeSystem() { return m_ComputeSystem; }

private:
	ControlProtocol::EntityManager& m_EntityManager;

	TransformHierarchySystem m_HierarchySystem;
	TransformDirtySystem m_DirtySystem;
	TransformComputeSystem m_ComputeSystem;
};

#endif