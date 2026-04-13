#pragma once
#ifndef __C_P_TRANSFORMMANAGER_H__
#define __C_P_TRANSFORMMANAGER_H__

#include <optional>
#include <unordered_map>
#include <vector>

#include "core/Entity/Entity.h"
#include "BaseProtocol/Transform/Transform.h"
#include "BaseProtocol/Transform/TransformState.h"
#include "ControlProtocol/EntityManager/EntityManager.h"
#include "DataProtocol/MathTypes.h"

namespace ControlProtocol
{
	class TransformManager
	{
	public:
		explicit TransformManager(EntityManager& entityManager);

	public:
		EZ::Entity CreateEntity(const DataProtocol::Vec3& position);

		void RegisterEntity(EZ::Entity entity);
		void UnregisterEntity(EZ::Entity entity);
		bool HasNode(EZ::Entity entity) const;

		bool EnableTransform(
			EZ::Entity entity,
			const DataProtocol::Transform& initial = DataProtocol::Transform{}
		);

		void DisableTransform(EZ::Entity entity);
		bool HasTransform(EZ::Entity entity) const;

		bool SetParent(
			EZ::Entity child,
			const std::optional<EZ::Entity>& parent
		);

		bool ClearParent(EZ::Entity child);
		std::optional<EZ::Entity> GetParent(EZ::Entity entity) const;

		void MarkWorldDirty(EZ::Entity entity);
		void MarkHierarchyDirty(EZ::Entity entity);

		const DataProtocol::Transform* TryGetLocalTransform(EZ::Entity entity) const;
		const DataProtocol::Mat4* TryGetLocalToWorld(EZ::Entity entity) const;
		const BaseProtocol::TransformState* TryGetTransformState(EZ::Entity entity) const;

		std::optional<EZ::Entity> GetFirstChild(EZ::Entity entity) const;
		std::optional<EZ::Entity> GetNextSibling(EZ::Entity entity) const;
		std::optional<EZ::Entity> GetPrevSibling(EZ::Entity entity) const;
		EZ::u32 GetDepth(EZ::Entity entity) const;

		void GetRootEntities(std::vector<EZ::Entity>& outRoots) const;
		void BuildHierarchyPreorder(std::vector<EZ::Entity>& outList) const;
		void BuildSubtreePreorder(
			EZ::Entity root,
			std::vector<EZ::Entity>& outList
		) const;

		void Update();
		void UpdateSubtree(EZ::Entity root);

		void GarbageCollectInvalidEntities();

	private:
		struct EntityHash
		{
			size_t operator()(EZ::Entity e) const noexcept
			{
				return static_cast<size_t>(static_cast<EZ::u32>(e));
			}
		};

		struct HierarchyNode
		{
			std::optional<EZ::Entity> parent;
			std::optional<EZ::Entity> firstChild;
			std::optional<EZ::Entity> nextSibling;
			std::optional<EZ::Entity> prevSibling;
			EZ::u32 depth = 0;
		};

	private:
		bool IsEntityValid(EZ::Entity entity) const;
		void EnsureNode(EZ::Entity entity);

		HierarchyNode* FindNode(EZ::Entity entity);
		const HierarchyNode* FindNode(EZ::Entity entity) const;

		void UnlinkNode(EZ::Entity entity);
		void LinkAsFirstChild(EZ::Entity parent, EZ::Entity child);
		void UpdateDepthRecursive(EZ::Entity entity, EZ::u32 depth);
		bool IsDescendantOf(
			EZ::Entity child,
			EZ::Entity potentialAncestor
		) const;

		void MarkSubtreeWorldDirty(EZ::Entity root);
		void MarkSubtreeHierarchyDirty(EZ::Entity root);

		void CollectLocalDirtyToWorldDirty();
		void UpdateNodeRecursive(
			EZ::Entity entity,
			const DataProtocol::Mat4& inheritedWorld,
			bool hasInheritedTransform
		);

		void BuildSubtreePreorderRecursive(
			EZ::Entity root,
			std::vector<EZ::Entity>& outList
		) const;

		void RemoveNodeInternal(EZ::Entity entity);

	private:
		EntityManager& m_EntityManager;
		std::unordered_map<EZ::Entity, HierarchyNode, EntityHash> m_Nodes;
	};
}

#endif