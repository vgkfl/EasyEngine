#pragma once
#ifndef __TRANSFORM_DIRTY_SYSTEM_H__
#define __TRANSFORM_DIRTY_SYSTEM_H__

#include "core/Entity/Entity.h"
#include "ControlProtocol/EntityManager/EntityManager.h"

class TransformDirtySystem
{
public:
	explicit TransformDirtySystem(ControlProtocol::EntityManager& entityManager);

public:
	void CollectLocalDirty();
	void PropagateWorldDirty();
	void ClearFrameDirty();

	void MarkWorldDirty(EZ::Entity entity);

private:
	void MarkSubtreeWorldDirty(EZ::Entity root);

private:
	ControlProtocol::EntityManager& m_EntityManager;
};

#endif