#include "System/TransformSystem/SubSystems/TransformDirtySystem.h"

#include "BaseProtocol/Transform/Transform.h"
#include "BaseProtocol/Transform/TransformHierarchy.h"

namespace
{
	static BaseProtocol::LocalTransform* TryGetLocalTransform(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		return entityManager.TryGetComponent<BaseProtocol::LocalTransform>(entity);
	}

	static BaseProtocol::TransformHierarchy* TryGetHierarchy(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		return entityManager.TryGetComponent<BaseProtocol::TransformHierarchy>(entity);
	}
}

TransformDirtySystem::TransformDirtySystem(ControlProtocol::EntityManager& entityManager)
	: m_EntityManager(entityManager)
{
}

void TransformDirtySystem::CollectLocalDirty()
{
	m_EntityManager.ForEach<BaseProtocol::LocalTransform>(
		[](EZ::Entity entity, BaseProtocol::LocalTransform& local)
		{
			(void)entity;

			if (local.IsLocalDirty())
			{
				local.MarkWorldDirty();
			}
		}
	);
}

void TransformDirtySystem::MarkSubtreeWorldDirty(EZ::Entity root)
{
	auto* local = TryGetLocalTransform(m_EntityManager, root);
	if (local)
	{
		local->MarkWorldDirty();
	}

	auto* hierarchy = TryGetHierarchy(m_EntityManager, root);
	if (!hierarchy)
	{
		return;
	}

	auto child = hierarchy->firstChild;
	while (child.has_value())
	{
		const EZ::Entity currentChild = *child;
		auto* childHierarchy = TryGetHierarchy(m_EntityManager, currentChild);
		auto next = childHierarchy ? childHierarchy->nextSibling : std::optional<EZ::Entity>{};

		MarkSubtreeWorldDirty(currentChild);
		child = next;
	}
}

void TransformDirtySystem::MarkWorldDirty(EZ::Entity entity)
{
	MarkSubtreeWorldDirty(entity);
}

void TransformDirtySystem::PropagateWorldDirty()
{
	m_EntityManager.ForEach<BaseProtocol::LocalTransform, BaseProtocol::TransformHierarchy>(
		[this](EZ::Entity entity, BaseProtocol::LocalTransform& local, BaseProtocol::TransformHierarchy& hierarchy)
		{
			if (local.IsLocalDirty() || hierarchy.hierarchyDirty || local.IsWorldDirty())
			{
				MarkSubtreeWorldDirty(entity);
			}
		}
	);
}

void TransformDirtySystem::ClearFrameDirty()
{
	m_EntityManager.ForEach<BaseProtocol::LocalTransform>(
		[](EZ::Entity entity, BaseProtocol::LocalTransform& local)
		{
			(void)entity;
			local.ClearLocalDirty();
			local.ClearWorldDirty();
		}
	);

	m_EntityManager.ForEach<BaseProtocol::TransformHierarchy>(
		[](EZ::Entity entity, BaseProtocol::TransformHierarchy& hierarchy)
		{
			(void)entity;
			hierarchy.hierarchyDirty = false;
		}
	);
}