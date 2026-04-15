#pragma once
#ifndef __TRANSFORM_HIERARCHY_SYSTEM_H__
#define __TRANSFORM_HIERARCHY_SYSTEM_H__

#include <optional>

#include "core/Entity/Entity.h"
#include "ControlProtocol/EntityManager/EntityManager.h"

class TransformHierarchySystem
{
public:
	explicit TransformHierarchySystem(ControlProtocol::EntityManager& entityManager);

public:
	bool SetParent(EZ::Entity child, const std::optional<EZ::Entity>& parent);
	bool ClearParent(EZ::Entity child);

	std::optional<EZ::Entity> GetParent(EZ::Entity entity) const;
	std::optional<EZ::Entity> GetFirstChild(EZ::Entity entity) const;
	std::optional<EZ::Entity> GetNextSibling(EZ::Entity entity) const;
	std::optional<EZ::Entity> GetPrevSibling(EZ::Entity entity) const;
	EZ::u32 GetDepth(EZ::Entity entity) const;

	void Flush();

private:
	bool IsValid(EZ::Entity entity) const;
	bool IsDescendantOf(EZ::Entity child, EZ::Entity ancestor) const;

	void UnlinkNode(EZ::Entity entity);
	void LinkAsFirstChild(EZ::Entity parent, EZ::Entity child);
	void UpdateDepthRecursive(EZ::Entity entity, EZ::u32 depth);

private:
	ControlProtocol::EntityManager& m_EntityManager;
};

#endif