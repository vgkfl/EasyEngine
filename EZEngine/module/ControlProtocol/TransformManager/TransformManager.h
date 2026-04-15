#pragma once
#ifndef __C_P_TRANSFORMMANAGER_H__
#define __C_P_TRANSFORMMANAGER_H__

#include <optional>
#include <vector>

#include "core/Entity/Entity.h"
#include "BaseProtocol/Transform/Transform.h"
#include "BaseProtocol/Transform/TransformHierarchy.h"
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

		// 兼容旧接口：TransformManager 不再自己执行 transform 更新
		void Update();
		void UpdateSubtree(EZ::Entity root);

		void GarbageCollectInvalidEntities();

	private:
		bool IsEntityValid(EZ::Entity entity) const;
		bool IsDescendantOf(
			EZ::Entity child,
			EZ::Entity potentialAncestor
		) const;

		void UnlinkNode(EZ::Entity entity);
		void LinkAsFirstChild(EZ::Entity parent, EZ::Entity child);
		void UpdateDepthRecursive(EZ::Entity entity, EZ::u32 depth);

		void MarkSubtreeWorldDirty(EZ::Entity root);
		void MarkSubtreeHierarchyDirty(EZ::Entity root);

		void BuildSubtreePreorderRecursive(
			EZ::Entity root,
			std::vector<EZ::Entity>& outList
		) const;

	private:
		EntityManager& m_EntityManager;
	};
}

#endif