#include "System/TransformSystem/SubSystems/TransformHierarchySystem.h"

#include "BaseProtocol/Transform/Transform.h"
#include "BaseProtocol/Transform/TransformHierarchy.h"

namespace
{
	static BaseProtocol::TransformHierarchy* TryGetHierarchy(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		return entityManager.TryGetComponent<BaseProtocol::TransformHierarchy>(entity);
	}

	static const BaseProtocol::TransformHierarchy* TryGetHierarchyConst(
		const ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		return const_cast<ControlProtocol::EntityManager&>(entityManager)
			.TryGetComponent<BaseProtocol::TransformHierarchy>(entity);
	}

	static BaseProtocol::TransformHierarchy& EnsureHierarchy(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		auto* hierarchy = entityManager.TryGetComponent<BaseProtocol::TransformHierarchy>(entity);
		if (hierarchy)
		{
			return *hierarchy;
		}
		return entityManager.AddComponent<BaseProtocol::TransformHierarchy>(entity);
	}

	static BaseProtocol::LocalTransform* TryGetLocalTransform(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		return entityManager.TryGetComponent<BaseProtocol::LocalTransform>(entity);
	}

	static void MarkWorldDirtyRecursive(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		if (auto* local = TryGetLocalTransform(entityManager, entity))
		{
			local->MarkWorldDirty();
		}

		auto* hierarchy = TryGetHierarchy(entityManager, entity);
		if (!hierarchy)
		{
			return;
		}

		auto child = hierarchy->firstChild;
		while (child.has_value())
		{
			const EZ::Entity currentChild = *child;
			auto* childHierarchy = TryGetHierarchy(entityManager, currentChild);
			auto next = childHierarchy ? childHierarchy->nextSibling : std::optional<EZ::Entity>{};

			MarkWorldDirtyRecursive(entityManager, currentChild);
			child = next;
		}
	}
}

TransformHierarchySystem::TransformHierarchySystem(ControlProtocol::EntityManager& entityManager)
	: m_EntityManager(entityManager)
{
}

bool TransformHierarchySystem::IsValid(EZ::Entity entity) const
{
	return m_EntityManager.IsValid(entity);
}

bool TransformHierarchySystem::IsDescendantOf(EZ::Entity child, EZ::Entity ancestor) const
{
	if (!IsValid(child) || !IsValid(ancestor))
	{
		return false;
	}

	auto* hierarchy = TryGetHierarchyConst(m_EntityManager, child);
	while (hierarchy && hierarchy->parent.has_value())
	{
		if (*hierarchy->parent == ancestor)
		{
			return true;
		}
		hierarchy = TryGetHierarchyConst(m_EntityManager, *hierarchy->parent);
	}

	return false;
}

void TransformHierarchySystem::UnlinkNode(EZ::Entity entity)
{
	auto* node = TryGetHierarchy(m_EntityManager, entity);
	if (!node)
	{
		return;
	}

	if (node->parent.has_value())
	{
		auto* parentNode = TryGetHierarchy(m_EntityManager, *node->parent);
		if (parentNode &&
			parentNode->firstChild.has_value() &&
			*parentNode->firstChild == entity)
		{
			parentNode->firstChild = node->nextSibling;
			parentNode->hierarchyDirty = true;
		}
	}

	if (node->prevSibling.has_value())
	{
		auto* prevNode = TryGetHierarchy(m_EntityManager, *node->prevSibling);
		if (prevNode)
		{
			prevNode->nextSibling = node->nextSibling;
			prevNode->hierarchyDirty = true;
		}
	}

	if (node->nextSibling.has_value())
	{
		auto* nextNode = TryGetHierarchy(m_EntityManager, *node->nextSibling);
		if (nextNode)
		{
			nextNode->prevSibling = node->prevSibling;
			nextNode->hierarchyDirty = true;
		}
	}

	node->parent.reset();
	node->prevSibling.reset();
	node->nextSibling.reset();
	node->depth = 0;
	node->hierarchyDirty = true;
}

void TransformHierarchySystem::LinkAsFirstChild(EZ::Entity parent, EZ::Entity child)
{
	auto& parentNode = EnsureHierarchy(m_EntityManager, parent);
	auto& childNode = EnsureHierarchy(m_EntityManager, child);

	childNode.parent = parent;
	childNode.prevSibling.reset();
	childNode.nextSibling = parentNode.firstChild;
	childNode.depth = parentNode.depth + 1;
	childNode.hierarchyDirty = true;

	if (parentNode.firstChild.has_value())
	{
		auto* oldFirstChild = TryGetHierarchy(m_EntityManager, *parentNode.firstChild);
		if (oldFirstChild)
		{
			oldFirstChild->prevSibling = child;
			oldFirstChild->hierarchyDirty = true;
		}
	}

	parentNode.firstChild = child;
	parentNode.hierarchyDirty = true;
}

void TransformHierarchySystem::UpdateDepthRecursive(EZ::Entity entity, EZ::u32 depth)
{
	auto* node = TryGetHierarchy(m_EntityManager, entity);
	if (!node)
	{
		return;
	}

	node->depth = depth;
	node->hierarchyDirty = true;

	auto child = node->firstChild;
	while (child.has_value())
	{
		const EZ::Entity currentChild = *child;
		auto* childNode = TryGetHierarchy(m_EntityManager, currentChild);
		auto next = childNode ? childNode->nextSibling : std::optional<EZ::Entity>{};

		UpdateDepthRecursive(currentChild, depth + 1);
		child = next;
	}
}

bool TransformHierarchySystem::SetParent(EZ::Entity child, const std::optional<EZ::Entity>& parent)
{
	if (!IsValid(child))
	{
		return false;
	}

	EnsureHierarchy(m_EntityManager, child);

	if (!parent.has_value())
	{
		return ClearParent(child);
	}

	if (!IsValid(*parent))
	{
		return false;
	}

	if (*parent == child)
	{
		return false;
	}

	EnsureHierarchy(m_EntityManager, *parent);

	// 不能把自己挂到自己的子孙下面
	if (IsDescendantOf(*parent, child))
	{
		return false;
	}

	auto* childNode = TryGetHierarchy(m_EntityManager, child);
	if (!childNode)
	{
		return false;
	}

	if (childNode->parent.has_value() && *childNode->parent == *parent)
	{
		return true;
	}

	UnlinkNode(child);
	LinkAsFirstChild(*parent, child);
	UpdateDepthRecursive(child, GetDepth(*parent) + 1);

	auto* parentNode = TryGetHierarchy(m_EntityManager, *parent);
	if (parentNode)
	{
		parentNode->hierarchyDirty = true;
	}

	MarkWorldDirtyRecursive(m_EntityManager, child);
	return true;
}

bool TransformHierarchySystem::ClearParent(EZ::Entity child)
{
	if (!IsValid(child))
	{
		return false;
	}

	auto* childNode = TryGetHierarchy(m_EntityManager, child);
	if (!childNode)
	{
		return false;
	}

	if (!childNode->parent.has_value())
	{
		UpdateDepthRecursive(child, 0);
		childNode->hierarchyDirty = true;
		return true;
	}

	UnlinkNode(child);
	UpdateDepthRecursive(child, 0);
	childNode->hierarchyDirty = true;

	MarkWorldDirtyRecursive(m_EntityManager, child);
	return true;
}

std::optional<EZ::Entity> TransformHierarchySystem::GetParent(EZ::Entity entity) const
{
	auto* node = TryGetHierarchyConst(m_EntityManager, entity);
	return node ? node->parent : std::optional<EZ::Entity>{};
}

std::optional<EZ::Entity> TransformHierarchySystem::GetFirstChild(EZ::Entity entity) const
{
	auto* node = TryGetHierarchyConst(m_EntityManager, entity);
	return node ? node->firstChild : std::optional<EZ::Entity>{};
}

std::optional<EZ::Entity> TransformHierarchySystem::GetNextSibling(EZ::Entity entity) const
{
	auto* node = TryGetHierarchyConst(m_EntityManager, entity);
	return node ? node->nextSibling : std::optional<EZ::Entity>{};
}

std::optional<EZ::Entity> TransformHierarchySystem::GetPrevSibling(EZ::Entity entity) const
{
	auto* node = TryGetHierarchyConst(m_EntityManager, entity);
	return node ? node->prevSibling : std::optional<EZ::Entity>{};
}

EZ::u32 TransformHierarchySystem::GetDepth(EZ::Entity entity) const
{
	auto* node = TryGetHierarchyConst(m_EntityManager, entity);
	return node ? node->depth : 0u;
}

void TransformHierarchySystem::Flush()
{
//	throw std::runtime_error("TransformHerarchySystem::Flush");
}