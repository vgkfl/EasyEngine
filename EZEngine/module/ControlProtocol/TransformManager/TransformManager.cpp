#include "ControlProtocol/TransformManager/TransformManager.h"

#include <stack>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace
{
	inline EZ::u32 ToMask(BaseProtocol::TransformFlags flags)
	{
		return static_cast<EZ::u32>(flags);
	}

	inline void SetFlag(BaseProtocol::TransformFlags& value, BaseProtocol::TransformFlags flag)
	{
		value = static_cast<BaseProtocol::TransformFlags>(ToMask(value) | ToMask(flag));
	}

	inline void ClearFlag(BaseProtocol::TransformFlags& value, BaseProtocol::TransformFlags flag)
	{
		value = static_cast<BaseProtocol::TransformFlags>(ToMask(value) & ~ToMask(flag));
	}

	inline glm::vec3 ToGLM(const DataProtocol::Vec3& v)
	{
		return glm::vec3(v.x, v.y, v.z);
	}

	inline glm::quat ToGLM(const DataProtocol::Quat& q)
	{
		return glm::quat(q.w, q.x, q.y, q.z);
	}

	inline DataProtocol::Vec3 FromGLM(const glm::vec3& v)
	{
		return DataProtocol::Vec3{ v.x, v.y, v.z };
	}

	inline DataProtocol::Quat FromGLM(const glm::quat& q)
	{
		return DataProtocol::Quat{ q.x, q.y, q.z, q.w };
	}

	inline glm::mat4 ToGLM(const DataProtocol::Mat4& m)
	{
		glm::mat4 result(1.0f);
		for (EZ::u32 i = 0; i < 16; ++i)
		{
			result[i / 4][i % 4] = m.m[i];
		}
		return result;
	}

	inline DataProtocol::Mat4 FromGLM(const glm::mat4& m)
	{
		DataProtocol::Mat4 result{};
		for (EZ::u32 i = 0; i < 16; ++i)
		{
			result.m[i] = m[i / 4][i % 4];
		}
		return result;
	}

	inline glm::mat4 ComposeLocalMatrix(const DataProtocol::Transform& t)
	{
		const glm::mat4 T = glm::translate(glm::mat4(1.0f), ToGLM(t.position));
		const glm::mat4 R = glm::toMat4(glm::normalize(ToGLM(t.rotation)));
		const glm::mat4 S = glm::scale(glm::mat4(1.0f), ToGLM(t.scale));
		return T * R * S;
	}

	inline glm::mat4 ComposeWorldWithInheritance(
		const glm::mat4& parentWorld,
		const DataProtocol::Transform& local,
		BaseProtocol::TransformFlags flags)
	{
		const bool inheritPos = BaseProtocol::HasFlag(flags, BaseProtocol::TransformFlags::InheritPosition);
		const bool inheritRot = BaseProtocol::HasFlag(flags, BaseProtocol::TransformFlags::InheritRotation);
		const bool inheritScl = BaseProtocol::HasFlag(flags, BaseProtocol::TransformFlags::InheritScale);

		if (inheritPos && inheritRot && inheritScl)
		{
			return parentWorld * ComposeLocalMatrix(local);
		}

		glm::vec3 skew(0.0f);
		glm::vec4 perspective(0.0f);
		glm::vec3 parentScale(1.0f);
		glm::quat parentRotation(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 parentTranslation(0.0f);

		glm::decompose(
			parentWorld,
			parentScale,
			parentRotation,
			parentTranslation,
			skew,
			perspective
		);

		DataProtocol::Transform inherited{};
		inherited.position = inheritPos ? FromGLM(parentTranslation) : DataProtocol::Vec3{};
		inherited.rotation = inheritRot ? FromGLM(glm::normalize(parentRotation)) : DataProtocol::Quat{};
		inherited.scale = inheritScl ? FromGLM(parentScale) : DataProtocol::Vec3{ 1.0f, 1.0f, 1.0f };

		return ComposeLocalMatrix(inherited) * ComposeLocalMatrix(local);
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
		auto entity = m_EntityManager.CreateEntity();
		RegisterEntity(entity);

		DataProtocol::Transform initial{};
		initial.position = position;
		initial.rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
		initial.scale = { 1.0f, 1.0f, 1.0f };

		EnableTransform(entity, initial);
		return entity;
	}

	bool TransformManager::IsEntityValid(EZ::Entity entity) const
	{
		return m_EntityManager.IsValid(entity);
	}

	void TransformManager::EnsureNode(EZ::Entity entity)
	{
		if (!IsEntityValid(entity))
		{
			return;
		}

		m_Nodes.try_emplace(entity);
	}

	bool TransformManager::HasNode(EZ::Entity entity) const
	{
		return m_Nodes.find(entity) != m_Nodes.end();
	}

	TransformManager::HierarchyNode* TransformManager::FindNode(EZ::Entity entity)
	{
		auto it = m_Nodes.find(entity);
		return (it == m_Nodes.end()) ? nullptr : &it->second;
	}

	const TransformManager::HierarchyNode* TransformManager::FindNode(EZ::Entity entity) const
	{
		auto it = m_Nodes.find(entity);
		return (it == m_Nodes.end()) ? nullptr : &it->second;
	}

	void TransformManager::RegisterEntity(EZ::Entity entity)
	{
		EnsureNode(entity);
	}

	void TransformManager::RemoveNodeInternal(EZ::Entity entity)
	{
		HierarchyNode* node = FindNode(entity);
		if (!node)
		{
			return;
		}

		auto child = node->firstChild;
		while (child.has_value())
		{
			HierarchyNode* childNode = FindNode(*child);
			auto next = childNode ? childNode->nextSibling : std::optional<EZ::Entity>{};

			if (childNode)
			{
				childNode->parent.reset();
				childNode->prevSibling.reset();
				childNode->nextSibling.reset();
				UpdateDepthRecursive(*child, 0);
			}

			child = next;
		}

		UnlinkNode(entity);
		m_Nodes.erase(entity);
	}

	void TransformManager::UnregisterEntity(EZ::Entity entity)
	{
		if (!HasNode(entity))
		{
			return;
		}

		RemoveNodeInternal(entity);
	}

	bool TransformManager::EnableTransform(
		EZ::Entity entity,
		const DataProtocol::Transform& initial)
	{
		if (!IsEntityValid(entity))
		{
			return false;
		}

		EnsureNode(entity);

		if (!m_EntityManager.HasComponent<BaseProtocol::LocalTransform>(entity))
		{
			m_EntityManager.AddComponent<BaseProtocol::LocalTransform>(entity);
		}

		if (!m_EntityManager.HasComponent<BaseProtocol::LocalToWorld>(entity))
		{
			m_EntityManager.AddComponent<BaseProtocol::LocalToWorld>(entity);
		}

		if (!m_EntityManager.HasComponent<BaseProtocol::TransformState>(entity))
		{
			m_EntityManager.AddComponent<BaseProtocol::TransformState>(entity);
		}

		auto& local = m_EntityManager.GetComponent<BaseProtocol::LocalTransform>(entity);
		local.Set(initial);

		auto& state = m_EntityManager.GetComponent<BaseProtocol::TransformState>(entity);
		SetFlag(state.flags, BaseProtocol::TransformFlags::WorldDirty);
		SetFlag(state.flags, BaseProtocol::TransformFlags::HierarchyDirty);

		MarkSubtreeHierarchyDirty(entity);

		return true;
	}

	void TransformManager::DisableTransform(EZ::Entity entity)
	{
		if (!IsEntityValid(entity))
		{
			return;
		}

		if (!HasTransform(entity))
		{
			return;
		}

		if (m_EntityManager.HasComponent<BaseProtocol::LocalTransform>(entity))
		{
			m_EntityManager.RemoveComponent<BaseProtocol::LocalTransform>(entity);
		}

		if (m_EntityManager.HasComponent<BaseProtocol::LocalToWorld>(entity))
		{
			m_EntityManager.RemoveComponent<BaseProtocol::LocalToWorld>(entity);
		}

		if (m_EntityManager.HasComponent<BaseProtocol::TransformState>(entity))
		{
			m_EntityManager.RemoveComponent<BaseProtocol::TransformState>(entity);
		}

		if (m_EntityManager.HasComponent<BaseProtocol::LocalMatrix>(entity))
		{
			m_EntityManager.RemoveComponent<BaseProtocol::LocalMatrix>(entity);
		}

		if (m_EntityManager.HasComponent<BaseProtocol::PreviousLocalToWorld>(entity))
		{
			m_EntityManager.RemoveComponent<BaseProtocol::PreviousLocalToWorld>(entity);
		}

		MarkSubtreeHierarchyDirty(entity);
	}

	bool TransformManager::HasTransform(EZ::Entity entity) const
	{
		if (!IsEntityValid(entity))
		{
			return false;
		}

		return
			m_EntityManager.HasComponent<BaseProtocol::LocalTransform>(entity) &&
			m_EntityManager.HasComponent<BaseProtocol::LocalToWorld>(entity) &&
			m_EntityManager.HasComponent<BaseProtocol::TransformState>(entity);
	}

	void TransformManager::UnlinkNode(EZ::Entity entity)
	{
		HierarchyNode* node = FindNode(entity);
		if (!node)
		{
			return;
		}

		if (node->parent.has_value())
		{
			HierarchyNode* parentNode = FindNode(*node->parent);
			if (parentNode &&
				parentNode->firstChild.has_value() &&
				*parentNode->firstChild == entity)
			{
				parentNode->firstChild = node->nextSibling;
			}
		}

		if (node->prevSibling.has_value())
		{
			HierarchyNode* prevNode = FindNode(*node->prevSibling);
			if (prevNode)
			{
				prevNode->nextSibling = node->nextSibling;
			}
		}

		if (node->nextSibling.has_value())
		{
			HierarchyNode* nextNode = FindNode(*node->nextSibling);
			if (nextNode)
			{
				nextNode->prevSibling = node->prevSibling;
			}
		}

		node->parent.reset();
		node->prevSibling.reset();
		node->nextSibling.reset();
		node->depth = 0;
	}

	void TransformManager::LinkAsFirstChild(EZ::Entity parent, EZ::Entity child)
	{
		HierarchyNode* parentNode = FindNode(parent);
		HierarchyNode* childNode = FindNode(child);

		if (!parentNode || !childNode)
		{
			return;
		}

		childNode->parent = parent;
		childNode->prevSibling.reset();
		childNode->nextSibling = parentNode->firstChild;

		if (parentNode->firstChild.has_value())
		{
			HierarchyNode* oldFirst = FindNode(*parentNode->firstChild);
			if (oldFirst)
			{
				oldFirst->prevSibling = child;
			}
		}

		parentNode->firstChild = child;
		UpdateDepthRecursive(child, parentNode->depth + 1);
	}

	void TransformManager::UpdateDepthRecursive(EZ::Entity entity, EZ::u32 depth)
	{
		HierarchyNode* node = FindNode(entity);
		if (!node)
		{
			return;
		}

		node->depth = depth;

		auto child = node->firstChild;
		while (child.has_value())
		{
			HierarchyNode* childNode = FindNode(*child);
			auto next = childNode ? childNode->nextSibling : std::optional<EZ::Entity>{};

			UpdateDepthRecursive(*child, depth + 1);
			child = next;
		}
	}

	bool TransformManager::IsDescendantOf(
		EZ::Entity child,
		EZ::Entity potentialAncestor
	) const
	{
		if (!IsEntityValid(child) || !IsEntityValid(potentialAncestor))
		{
			return false;
		}

		std::optional<EZ::Entity> current = child;

		while (current.has_value())
		{
			if (*current == potentialAncestor)
			{
				return true;
			}

			const HierarchyNode* node = FindNode(*current);
			if (!node)
			{
				return false;
			}

			current = node->parent;
		}

		return false;
	}

	bool TransformManager::SetParent(
		EZ::Entity child,
		const std::optional<EZ::Entity>& parent
	)
	{
		if (!IsEntityValid(child))
		{
			return false;
		}

		if (parent.has_value())
		{
			if (!IsEntityValid(*parent))
			{
				return false;
			}

			if (*parent == child)
			{
				return false;
			}
		}

		EnsureNode(child);

		if (parent.has_value())
		{
			EnsureNode(*parent);

			if (IsDescendantOf(*parent, child))
			{
				return false;
			}
		}

		UnlinkNode(child);

		if (parent.has_value())
		{
			LinkAsFirstChild(*parent, child);
		}
		else
		{
			UpdateDepthRecursive(child, 0);
		}

		MarkSubtreeHierarchyDirty(child);
		return true;
	}

	bool TransformManager::ClearParent(EZ::Entity child)
	{
		if (!IsEntityValid(child))
		{
			return false;
		}

		HierarchyNode* node = FindNode(child);
		if (!node)
		{
			return false;
		}

		UnlinkNode(child);
		UpdateDepthRecursive(child, 0);
		MarkSubtreeHierarchyDirty(child);
		return true;
	}

	std::optional<EZ::Entity> TransformManager::GetParent(EZ::Entity entity) const
	{
		const HierarchyNode* node = FindNode(entity);
		return node ? node->parent : std::optional<EZ::Entity>{};
	}

	void TransformManager::MarkSubtreeWorldDirty(EZ::Entity root)
	{
		if (!HasNode(root))
		{
			return;
		}

		std::stack<EZ::Entity> stack;
		stack.push(root);

		while (!stack.empty())
		{
			const EZ::Entity current = stack.top();
			stack.pop();

			if (HasTransform(current))
			{
				auto& state = m_EntityManager.GetComponent<BaseProtocol::TransformState>(current);
				SetFlag(state.flags, BaseProtocol::TransformFlags::WorldDirty);
			}

			const HierarchyNode* node = FindNode(current);
			if (!node)
			{
				continue;
			}

			auto child = node->firstChild;
			while (child.has_value())
			{
				const HierarchyNode* childNode = FindNode(*child);
				auto next = childNode ? childNode->nextSibling : std::optional<EZ::Entity>{};

				stack.push(*child);
				child = next;
			}
		}
	}

	void TransformManager::MarkSubtreeHierarchyDirty(EZ::Entity root)
	{
		if (!HasNode(root))
		{
			return;
		}

		std::stack<EZ::Entity> stack;
		stack.push(root);

		while (!stack.empty())
		{
			const EZ::Entity current = stack.top();
			stack.pop();

			if (HasTransform(current))
			{
				auto& state = m_EntityManager.GetComponent<BaseProtocol::TransformState>(current);
				SetFlag(state.flags, BaseProtocol::TransformFlags::HierarchyDirty);
				SetFlag(state.flags, BaseProtocol::TransformFlags::WorldDirty);
			}

			const HierarchyNode* node = FindNode(current);
			if (!node)
			{
				continue;
			}

			auto child = node->firstChild;
			while (child.has_value())
			{
				const HierarchyNode* childNode = FindNode(*child);
				auto next = childNode ? childNode->nextSibling : std::optional<EZ::Entity>{};

				stack.push(*child);
				child = next;
			}
		}
	}

	void TransformManager::MarkWorldDirty(EZ::Entity entity)
	{
		MarkSubtreeWorldDirty(entity);
	}

	void TransformManager::MarkHierarchyDirty(EZ::Entity entity)
	{
		MarkSubtreeHierarchyDirty(entity);
	}

	const DataProtocol::Transform* TransformManager::TryGetLocalTransform(EZ::Entity entity) const
	{
		if (!HasTransform(entity))
		{
			return nullptr;
		}

		const auto& local = m_EntityManager.GetComponent<BaseProtocol::LocalTransform>(entity);
		return &local.Read();
	}

	const DataProtocol::Mat4* TransformManager::TryGetLocalToWorld(EZ::Entity entity) const
	{
		if (!HasTransform(entity))
		{
			return nullptr;
		}

		const auto& world = m_EntityManager.GetComponent<BaseProtocol::LocalToWorld>(entity);
		return &world.value;
	}

	const BaseProtocol::TransformState* TransformManager::TryGetTransformState(EZ::Entity entity) const
	{
		if (!HasTransform(entity))
		{
			return nullptr;
		}

		const auto& state = m_EntityManager.GetComponent<BaseProtocol::TransformState>(entity);
		return &state;
	}

	std::optional<EZ::Entity> TransformManager::GetFirstChild(EZ::Entity entity) const
	{
		const HierarchyNode* node = FindNode(entity);
		return node ? node->firstChild : std::optional<EZ::Entity>{};
	}

	std::optional<EZ::Entity> TransformManager::GetNextSibling(EZ::Entity entity) const
	{
		const HierarchyNode* node = FindNode(entity);
		return node ? node->nextSibling : std::optional<EZ::Entity>{};
	}

	std::optional<EZ::Entity> TransformManager::GetPrevSibling(EZ::Entity entity) const
	{
		const HierarchyNode* node = FindNode(entity);
		return node ? node->prevSibling : std::optional<EZ::Entity>{};
	}

	EZ::u32 TransformManager::GetDepth(EZ::Entity entity) const
	{
		const HierarchyNode* node = FindNode(entity);
		return node ? node->depth : 0;
	}

	void TransformManager::GetRootEntities(std::vector<EZ::Entity>& outRoots) const
	{
		outRoots.clear();
		outRoots.reserve(m_Nodes.size());

		for (const auto& [entity, node] : m_Nodes)
		{
			if (!IsEntityValid(entity))
			{
				continue;
			}

			if (!node.parent.has_value())
			{
				outRoots.push_back(entity);
			}
		}
	}

	void TransformManager::BuildSubtreePreorderRecursive(
		EZ::Entity root,
		std::vector<EZ::Entity>& outList
	) const
	{
		if (!IsEntityValid(root))
		{
			return;
		}

		outList.push_back(root);

		const HierarchyNode* node = FindNode(root);
		if (!node)
		{
			return;
		}

		auto child = node->firstChild;
		while (child.has_value())
		{
			const HierarchyNode* childNode = FindNode(*child);
			auto next = childNode ? childNode->nextSibling : std::optional<EZ::Entity>{};

			BuildSubtreePreorderRecursive(*child, outList);
			child = next;
		}
	}

	void TransformManager::BuildHierarchyPreorder(std::vector<EZ::Entity>& outList) const
	{
		outList.clear();

		std::vector<EZ::Entity> roots;
		GetRootEntities(roots);

		for (const auto& root : roots)
		{
			BuildSubtreePreorderRecursive(root, outList);
		}
	}

	void TransformManager::BuildSubtreePreorder(
		EZ::Entity root,
		std::vector<EZ::Entity>& outList
	) const
	{
		outList.clear();
		BuildSubtreePreorderRecursive(root, outList);
	}

	void TransformManager::CollectLocalDirtyToWorldDirty()
	{
		for (auto& [entity, node] : m_Nodes)
		{
			(void)node;

			if (!HasTransform(entity))
			{
				continue;
			}

			auto& local = m_EntityManager.GetComponent<BaseProtocol::LocalTransform>(entity);
			if (!local.IsLocalDirty())
			{
				continue;
			}

			local.ClearLocalDirty();
			MarkSubtreeWorldDirty(entity);
		}
	}

	void TransformManager::UpdateNodeRecursive(
		EZ::Entity entity,
		const DataProtocol::Mat4& inheritedWorld,
		bool hasInheritedTransform
	)
	{
		const HierarchyNode* node = FindNode(entity);
		if (!node)
		{
			return;
		}

		DataProtocol::Mat4 currentWorld = inheritedWorld;
		bool currentHasTransform = hasInheritedTransform;

		if (HasTransform(entity))
		{
			auto& local = m_EntityManager.GetComponent<BaseProtocol::LocalTransform>(entity);
			auto& localToWorld = m_EntityManager.GetComponent<BaseProtocol::LocalToWorld>(entity);
			auto& state = m_EntityManager.GetComponent<BaseProtocol::TransformState>(entity);

			const bool needUpdate =
				BaseProtocol::HasFlag(state.flags, BaseProtocol::TransformFlags::WorldDirty) ||
				BaseProtocol::HasFlag(state.flags, BaseProtocol::TransformFlags::HierarchyDirty);

			if (needUpdate)
			{
				const glm::mat4 localMatrix = ComposeLocalMatrix(local.Read());

				if (m_EntityManager.HasComponent<BaseProtocol::PreviousLocalToWorld>(entity))
				{
					auto& prev = m_EntityManager.GetComponent<BaseProtocol::PreviousLocalToWorld>(entity);
					prev.value = localToWorld.value;
				}

				if (m_EntityManager.HasComponent<BaseProtocol::LocalMatrix>(entity))
				{
					auto& cachedLocal = m_EntityManager.GetComponent<BaseProtocol::LocalMatrix>(entity);
					cachedLocal.value = FromGLM(localMatrix);
				}

				glm::mat4 worldMatrix = localMatrix;

				if (hasInheritedTransform)
				{
					worldMatrix = ComposeWorldWithInheritance(
						ToGLM(inheritedWorld),
						local.Read(),
						state.flags
					);
				}

				localToWorld.value = FromGLM(worldMatrix);

				ClearFlag(state.flags, BaseProtocol::TransformFlags::WorldDirty);
				ClearFlag(state.flags, BaseProtocol::TransformFlags::HierarchyDirty);
			}

			currentWorld = localToWorld.value;
			currentHasTransform = true;
		}

		auto child = node->firstChild;
		while (child.has_value())
		{
			const HierarchyNode* childNode = FindNode(*child);
			auto next = childNode ? childNode->nextSibling : std::optional<EZ::Entity>{};

			UpdateNodeRecursive(*child, currentWorld, currentHasTransform);
			child = next;
		}
	}

	void TransformManager::GarbageCollectInvalidEntities()
	{
		std::vector<EZ::Entity> invalidEntities;
		invalidEntities.reserve(m_Nodes.size());

		for (const auto& [entity, node] : m_Nodes)
		{
			(void)node;
			if (!IsEntityValid(entity))
			{
				invalidEntities.push_back(entity);
			}
		}

		for (const auto& entity : invalidEntities)
		{
			RemoveNodeInternal(entity);
		}
	}

	void TransformManager::Update()
	{
		GarbageCollectInvalidEntities();
		CollectLocalDirtyToWorldDirty();

		const DataProtocol::Mat4 identity = FromGLM(glm::mat4(1.0f));

		for (const auto& [entity, node] : m_Nodes)
		{
			if (!IsEntityValid(entity))
			{
				continue;
			}

			if (node.parent.has_value())
			{
				continue;
			}

			UpdateNodeRecursive(entity, identity, false);
		}
	}

	void TransformManager::UpdateSubtree(EZ::Entity root)
	{
		if (!HasNode(root) || !IsEntityValid(root))
		{
			return;
		}

		GarbageCollectInvalidEntities();
		CollectLocalDirtyToWorldDirty();

		const auto parent = GetParent(root);

		if (!parent.has_value() || !HasTransform(*parent))
		{
			const DataProtocol::Mat4 identity = FromGLM(glm::mat4(1.0f));
			UpdateNodeRecursive(root, identity, false);
			return;
		}

		const auto* parentWorld = TryGetLocalToWorld(*parent);
		if (!parentWorld)
		{
			const DataProtocol::Mat4 identity = FromGLM(glm::mat4(1.0f));
			UpdateNodeRecursive(root, identity, false);
			return;
		}

		UpdateNodeRecursive(root, *parentWorld, true);
	}
}