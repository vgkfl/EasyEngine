#include "ControlProtocol/TransformManager/TransformManager.h"

#include <algorithm>

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

	static BaseProtocol::LocalTransform* TryGetLocalTransformMutable(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		return entityManager.TryGetComponent<BaseProtocol::LocalTransform>(entity);
	}

	static const BaseProtocol::LocalTransform* TryGetLocalTransformConst(
		const ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		return const_cast<ControlProtocol::EntityManager&>(entityManager)
			.TryGetComponent<BaseProtocol::LocalTransform>(entity);
	}

	static BaseProtocol::LocalToWorld* TryGetLocalToWorldMutable(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		return entityManager.TryGetComponent<BaseProtocol::LocalToWorld>(entity);
	}

	static const BaseProtocol::LocalToWorld* TryGetLocalToWorldConst(
		const ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		return const_cast<ControlProtocol::EntityManager&>(entityManager)
			.TryGetComponent<BaseProtocol::LocalToWorld>(entity);
	}
}

namespace ControlProtocol
{
	TransformManager::TransformManager(EntityManager& entityManager)
		: m_EntityManager(entityManager)
	{
	}

	EZ::Entity TransformManager::CreateEntity(const DataProtocol::Vec3& position)
	{
		const EZ::Entity entity = m_EntityManager.CreateEntity();

		DataProtocol::Transform initial{};
		initial.position = position;
		initial.rotation = DataProtocol::Quat{};
		initial.scale = DataProtocol::Vec3{ 1.0f, 1.0f, 1.0f };

		RegisterEntity(entity);
		EnableTransform(entity, initial);
		return entity;
	}

	bool TransformManager::IsEntityValid(EZ::Entity entity) const
	{
		return m_EntityManager.IsValid(entity);
	}

	void TransformManager::RegisterEntity(EZ::Entity entity)
	{
		if (!IsEntityValid(entity))
		{
			return;
		}

		EnsureHierarchy(m_EntityManager, entity);
	}

	void TransformManager::UnregisterEntity(EZ::Entity entity)
	{
		if (!IsEntityValid(entity))
		{
			return;
		}

		if (!m_EntityManager.HasComponent<BaseProtocol::TransformHierarchy>(entity))
		{
			return;
		}

		ClearParent(entity);

		auto* hierarchy = TryGetHierarchy(m_EntityManager, entity);
		if (hierarchy)
		{
			auto child = hierarchy->firstChild;
			while (child.has_value())
			{
				const EZ::Entity currentChild = *child;
				auto* childHierarchy = TryGetHierarchy(m_EntityManager, currentChild);
				auto next = childHierarchy ? childHierarchy->nextSibling : std::optional<EZ::Entity>{};

				if (childHierarchy)
				{
					childHierarchy->parent.reset();
					childHierarchy->prevSibling.reset();
					childHierarchy->nextSibling.reset();
					childHierarchy->depth = 0;
					childHierarchy->hierarchyDirty = true;
				}

				if (auto* childLocal = TryGetLocalTransformMutable(m_EntityManager, currentChild))
				{
					childLocal->MarkWorldDirty();
				}

				UpdateDepthRecursive(currentChild, 0);
				child = next;
			}

			hierarchy->firstChild.reset();
		}

		m_EntityManager.RemoveComponent<BaseProtocol::TransformHierarchy>(entity);
	}

	bool TransformManager::HasNode(EZ::Entity entity) const
	{
		return IsEntityValid(entity) &&
			m_EntityManager.HasComponent<BaseProtocol::TransformHierarchy>(entity);
	}

	bool TransformManager::EnableTransform(
		EZ::Entity entity,
		const DataProtocol::Transform& initial)
	{
		if (!IsEntityValid(entity))
		{
			return false;
		}

		RegisterEntity(entity);

		if (!m_EntityManager.HasComponent<BaseProtocol::LocalTransform>(entity))
		{
			m_EntityManager.AddComponent<BaseProtocol::LocalTransform>(entity);
		}

		if (!m_EntityManager.HasComponent<BaseProtocol::LocalMatrix>(entity))
		{
			m_EntityManager.AddComponent<BaseProtocol::LocalMatrix>(entity);
		}

		if (!m_EntityManager.HasComponent<BaseProtocol::LocalToWorld>(entity))
		{
			m_EntityManager.AddComponent<BaseProtocol::LocalToWorld>(entity);
		}

		if (!m_EntityManager.HasComponent<BaseProtocol::PreviousLocalToWorld>(entity))
		{
			m_EntityManager.AddComponent<BaseProtocol::PreviousLocalToWorld>(entity);
		}

		if (!m_EntityManager.HasComponent<BaseProtocol::TransformHierarchy>(entity))
		{
			m_EntityManager.AddComponent<BaseProtocol::TransformHierarchy>(entity);
		}

		auto& local = m_EntityManager.GetComponent<BaseProtocol::LocalTransform>(entity);
		local.Set(initial);

		auto& hierarchy = m_EntityManager.GetComponent<BaseProtocol::TransformHierarchy>(entity);
		hierarchy.hierarchyDirty = true;

		return true;
	}

	void TransformManager::DisableTransform(EZ::Entity entity)
	{
		if (!IsEntityValid(entity))
		{
			return;
		}

		if (m_EntityManager.HasComponent<BaseProtocol::TransformHierarchy>(entity))
		{
			ClearParent(entity);

			auto* hierarchy = TryGetHierarchy(m_EntityManager, entity);
			if (hierarchy)
			{
				auto child = hierarchy->firstChild;
				while (child.has_value())
				{
					const EZ::Entity currentChild = *child;
					auto* childHierarchy = TryGetHierarchy(m_EntityManager, currentChild);
					auto next = childHierarchy ? childHierarchy->nextSibling : std::optional<EZ::Entity>{};

					if (childHierarchy)
					{
						childHierarchy->parent.reset();
						childHierarchy->prevSibling.reset();
						childHierarchy->nextSibling.reset();
						childHierarchy->depth = 0;
						childHierarchy->hierarchyDirty = true;
					}

					if (auto* childLocal = TryGetLocalTransformMutable(m_EntityManager, currentChild))
					{
						childLocal->MarkWorldDirty();
					}

					UpdateDepthRecursive(currentChild, 0);
					child = next;
				}
			}

			m_EntityManager.RemoveComponent<BaseProtocol::TransformHierarchy>(entity);
		}

		if (m_EntityManager.HasComponent<BaseProtocol::PreviousLocalToWorld>(entity))
		{
			m_EntityManager.RemoveComponent<BaseProtocol::PreviousLocalToWorld>(entity);
		}

		if (m_EntityManager.HasComponent<BaseProtocol::LocalToWorld>(entity))
		{
			m_EntityManager.RemoveComponent<BaseProtocol::LocalToWorld>(entity);
		}

		if (m_EntityManager.HasComponent<BaseProtocol::LocalMatrix>(entity))
		{
			m_EntityManager.RemoveComponent<BaseProtocol::LocalMatrix>(entity);
		}

		if (m_EntityManager.HasComponent<BaseProtocol::LocalTransform>(entity))
		{
			m_EntityManager.RemoveComponent<BaseProtocol::LocalTransform>(entity);
		}
	}

	bool TransformManager::HasTransform(EZ::Entity entity) const
	{
		return IsEntityValid(entity) &&
			m_EntityManager.HasComponent<BaseProtocol::LocalTransform>(entity) &&
			m_EntityManager.HasComponent<BaseProtocol::LocalMatrix>(entity) &&
			m_EntityManager.HasComponent<BaseProtocol::LocalToWorld>(entity) &&
			m_EntityManager.HasComponent<BaseProtocol::PreviousLocalToWorld>(entity) &&
			m_EntityManager.HasComponent<BaseProtocol::TransformHierarchy>(entity);
	}

	void TransformManager::UnlinkNode(EZ::Entity entity)
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

	void TransformManager::LinkAsFirstChild(EZ::Entity parent, EZ::Entity child)
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

	void TransformManager::UpdateDepthRecursive(EZ::Entity entity, EZ::u32 depth)
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

	bool TransformManager::IsDescendantOf(
		EZ::Entity child,
		EZ::Entity potentialAncestor) const
	{
		if (!IsEntityValid(child) || !IsEntityValid(potentialAncestor))
		{
			return false;
		}

		auto* hierarchy = TryGetHierarchyConst(m_EntityManager, child);
		while (hierarchy && hierarchy->parent.has_value())
		{
			if (*hierarchy->parent == potentialAncestor)
			{
				return true;
			}
			hierarchy = TryGetHierarchyConst(m_EntityManager, *hierarchy->parent);
		}

		return false;
	}

	bool TransformManager::SetParent(
		EZ::Entity child,
		const std::optional<EZ::Entity>& parent)
	{
		if (!IsEntityValid(child))
		{
			return false;
		}

		RegisterEntity(child);

		if (!parent.has_value())
		{
			return ClearParent(child);
		}

		if (!IsEntityValid(*parent))
		{
			return false;
		}

		if (*parent == child)
		{
			return false;
		}

		RegisterEntity(*parent);

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

		if (auto* parentNode = TryGetHierarchy(m_EntityManager, *parent))
		{
			parentNode->hierarchyDirty = true;
		}

		MarkSubtreeWorldDirty(child);
		return true;
	}

	bool TransformManager::ClearParent(EZ::Entity child)
	{
		if (!IsEntityValid(child))
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

		MarkSubtreeWorldDirty(child);
		return true;
	}

	std::optional<EZ::Entity> TransformManager::GetParent(EZ::Entity entity) const
	{
		auto* node = TryGetHierarchyConst(m_EntityManager, entity);
		return node ? node->parent : std::optional<EZ::Entity>{};
	}

	void TransformManager::MarkSubtreeWorldDirty(EZ::Entity root)
	{
		auto* local = TryGetLocalTransformMutable(m_EntityManager, root);
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

	void TransformManager::MarkSubtreeHierarchyDirty(EZ::Entity root)
	{
		auto* hierarchy = TryGetHierarchy(m_EntityManager, root);
		if (hierarchy)
		{
			hierarchy->hierarchyDirty = true;
		}

		if (auto* local = TryGetLocalTransformMutable(m_EntityManager, root))
		{
			local->MarkWorldDirty();
		}

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

			MarkSubtreeHierarchyDirty(currentChild);
			child = next;
		}
	}

	void TransformManager::MarkWorldDirty(EZ::Entity entity)
	{
		if (!IsEntityValid(entity))
		{
			return;
		}

		MarkSubtreeWorldDirty(entity);
	}

	void TransformManager::MarkHierarchyDirty(EZ::Entity entity)
	{
		if (!IsEntityValid(entity))
		{
			return;
		}

		MarkSubtreeHierarchyDirty(entity);
	}

	const DataProtocol::Transform* TransformManager::TryGetLocalTransform(EZ::Entity entity) const
	{
		const auto* local = TryGetLocalTransformConst(m_EntityManager, entity);
		return local ? &local->Read() : nullptr;
	}

	const DataProtocol::Mat4* TransformManager::TryGetLocalToWorld(EZ::Entity entity) const
	{
		const auto* localToWorld = TryGetLocalToWorldConst(m_EntityManager, entity);
		return localToWorld ? &localToWorld->value : nullptr;
	}

	std::optional<EZ::Entity> TransformManager::GetFirstChild(EZ::Entity entity) const
	{
		auto* node = TryGetHierarchyConst(m_EntityManager, entity);
		return node ? node->firstChild : std::optional<EZ::Entity>{};
	}

	std::optional<EZ::Entity> TransformManager::GetNextSibling(EZ::Entity entity) const
	{
		auto* node = TryGetHierarchyConst(m_EntityManager, entity);
		return node ? node->nextSibling : std::optional<EZ::Entity>{};
	}

	std::optional<EZ::Entity> TransformManager::GetPrevSibling(EZ::Entity entity) const
	{
		auto* node = TryGetHierarchyConst(m_EntityManager, entity);
		return node ? node->prevSibling : std::optional<EZ::Entity>{};
	}

	EZ::u32 TransformManager::GetDepth(EZ::Entity entity) const
	{
		auto* node = TryGetHierarchyConst(m_EntityManager, entity);
		return node ? node->depth : 0u;
	}

	void TransformManager::GetRootEntities(std::vector<EZ::Entity>& outRoots) const
	{
		outRoots.clear();

		auto& entityManager = const_cast<EntityManager&>(m_EntityManager);
		entityManager.ForEach<BaseProtocol::TransformHierarchy>(
			[this, &outRoots](EZ::Entity entity, BaseProtocol::TransformHierarchy& hierarchy)
			{
				bool isRoot = !hierarchy.parent.has_value();

				if (hierarchy.parent.has_value())
				{
					const EZ::Entity parent = *hierarchy.parent;
					if (!IsEntityValid(parent) || !HasNode(parent))
					{
						isRoot = true;
					}
				}

				if (isRoot)
				{
					outRoots.push_back(entity);
				}
			}
		);

		std::sort(
			outRoots.begin(),
			outRoots.end(),
			[](EZ::Entity a, EZ::Entity b)
			{
				return static_cast<EZ::u32>(a) < static_cast<EZ::u32>(b);
			});
	}

	void TransformManager::BuildSubtreePreorderRecursive(
		EZ::Entity root,
		std::vector<EZ::Entity>& outList) const
	{
		if (!IsEntityValid(root))
		{
			return;
		}

		outList.push_back(root);

		auto child = GetFirstChild(root);
		while (child.has_value())
		{
			const EZ::Entity currentChild = *child;
			auto next = GetNextSibling(currentChild);

			BuildSubtreePreorderRecursive(currentChild, outList);
			child = next;
		}
	}

	void TransformManager::BuildHierarchyPreorder(std::vector<EZ::Entity>& outList) const
	{
		outList.clear();

		std::vector<EZ::Entity> roots;
		GetRootEntities(roots);

		for (EZ::Entity root : roots)
		{
			BuildSubtreePreorderRecursive(root, outList);
		}
	}

	void TransformManager::BuildSubtreePreorder(
		EZ::Entity root,
		std::vector<EZ::Entity>& outList) const
	{
		outList.clear();

		if (!IsEntityValid(root))
		{
			return;
		}

		if (!HasNode(root))
		{
			outList.push_back(root);
			return;
		}

		BuildSubtreePreorderRecursive(root, outList);
	}

	void TransformManager::Update()
	{
		// 新规下 transform 统一由 TransformSystem::LateUpdate() 结算。
	}

	void TransformManager::UpdateSubtree(EZ::Entity root)
	{
		if (!IsEntityValid(root))
		{
			return;
		}

		// 兼容旧接口：这里只保留“打脏”语义，不做即时矩阵计算。
		MarkSubtreeWorldDirty(root);
	}

	void TransformManager::GarbageCollectInvalidEntities()
	{
		// 新实现没有内部 m_Nodes，不需要额外清理。
	}
}