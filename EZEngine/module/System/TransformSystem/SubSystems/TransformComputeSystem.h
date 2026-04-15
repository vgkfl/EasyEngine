#pragma once
#ifndef __TRANSFORM_COMPUTE_SYSTEM_H__
#define __TRANSFORM_COMPUTE_SYSTEM_H__

#include "core/Entity/Entity.h"
#include "ControlProtocol/EntityManager/EntityManager.h"
#include "DataProtocol/MathTypes.h"

class TransformComputeSystem
{
public:
	explicit TransformComputeSystem(ControlProtocol::EntityManager& entityManager);

public:
	void UpdateWorldTransforms();

private:
	void UpdateNodeRecursive(
		EZ::Entity entity,
		const DataProtocol::Mat4& parentWorld,
		bool hasParentWorld);

private:
	ControlProtocol::EntityManager& m_EntityManager;
};

#endif