#include "System/TransformSystem/SubSystems/TransformComputeSystem.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include "BaseProtocol/Transform/Transform.h"
#include "BaseProtocol/Transform/TransformHierarchy.h"

namespace
{
	static glm::vec3 ToGLM(const DataProtocol::Vec3& v)
	{
		return glm::vec3(v.x, v.y, v.z);
	}

	static glm::quat ToGLM(const DataProtocol::Quat& q)
	{
		return glm::quat(q.w, q.x, q.y, q.z);
	}

	static DataProtocol::Vec3 FromGLM(const glm::vec3& v)
	{
		return DataProtocol::Vec3{ v.x, v.y, v.z };
	}

	static DataProtocol::Quat FromGLM(const glm::quat& q)
	{
		return DataProtocol::Quat{ q.x, q.y, q.z, q.w };
	}

	static glm::mat4 ToGLM(const DataProtocol::Mat4& m)
	{
		glm::mat4 result(1.0f);
		for (EZ::u32 i = 0; i < 16; ++i)
		{
			result[i / 4][i % 4] = m.m[i];
		}
		return result;
	}

	static DataProtocol::Mat4 FromGLM(const glm::mat4& m)
	{
		DataProtocol::Mat4 result{};
		for (EZ::u32 i = 0; i < 16; ++i)
		{
			result.m[i] = m[i / 4][i % 4];
		}
		return result;
	}

	static glm::mat4 ComposeLocalMatrix(const DataProtocol::Transform& t)
	{
		const glm::mat4 T = glm::translate(glm::mat4(1.0f), ToGLM(t.position));
		const glm::mat4 R = glm::toMat4(glm::normalize(ToGLM(t.rotation)));
		const glm::mat4 S = glm::scale(glm::mat4(1.0f), ToGLM(t.scale));
		return T * R * S;
	}

	static glm::mat4 ComposeWorldWithInheritance(
		const glm::mat4& parentWorld,
		const DataProtocol::Transform& local,
		const BaseProtocol::TransformHierarchy& hierarchy)
	{
		const bool inheritPos = hierarchy.inheritPosition;
		const bool inheritRot = hierarchy.inheritRotation;
		const bool inheritScl = hierarchy.inheritScale;

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

	static BaseProtocol::LocalTransform* TryGetLocalTransform(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		return entityManager.TryGetComponent<BaseProtocol::LocalTransform>(entity);
	}

	static BaseProtocol::LocalMatrix* TryGetLocalMatrix(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		return entityManager.TryGetComponent<BaseProtocol::LocalMatrix>(entity);
	}

	static BaseProtocol::LocalToWorld* TryGetLocalToWorld(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		return entityManager.TryGetComponent<BaseProtocol::LocalToWorld>(entity);
	}

	static BaseProtocol::PreviousLocalToWorld* TryGetPreviousLocalToWorld(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		return entityManager.TryGetComponent<BaseProtocol::PreviousLocalToWorld>(entity);
	}

	static BaseProtocol::TransformHierarchy* TryGetHierarchy(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		return entityManager.TryGetComponent<BaseProtocol::TransformHierarchy>(entity);
	}

	static BaseProtocol::LocalMatrix& EnsureLocalMatrix(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		auto* value = TryGetLocalMatrix(entityManager, entity);
		if (value)
		{
			return *value;
		}
		return entityManager.AddComponent<BaseProtocol::LocalMatrix>(entity);
	}

	static BaseProtocol::LocalToWorld& EnsureLocalToWorld(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		auto* value = TryGetLocalToWorld(entityManager, entity);
		if (value)
		{
			return *value;
		}
		return entityManager.AddComponent<BaseProtocol::LocalToWorld>(entity);
	}

	static BaseProtocol::PreviousLocalToWorld& EnsurePreviousLocalToWorld(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		auto* value = TryGetPreviousLocalToWorld(entityManager, entity);
		if (value)
		{
			return *value;
		}
		return entityManager.AddComponent<BaseProtocol::PreviousLocalToWorld>(entity);
	}
}

TransformComputeSystem::TransformComputeSystem(ControlProtocol::EntityManager& entityManager)
	: m_EntityManager(entityManager)
{
}

void TransformComputeSystem::UpdateNodeRecursive(
	EZ::Entity entity,
	const DataProtocol::Mat4& parentWorld,
	bool hasParentWorld)
{
	auto* local = TryGetLocalTransform(m_EntityManager, entity);
	auto* hierarchy = TryGetHierarchy(m_EntityManager, entity);

	if (!local || !hierarchy)
	{
		return;
	}

	BaseProtocol::LocalMatrix& localMatrix = EnsureLocalMatrix(m_EntityManager, entity);
	BaseProtocol::LocalToWorld& localToWorld = EnsureLocalToWorld(m_EntityManager, entity);
	BaseProtocol::PreviousLocalToWorld& prevLocalToWorld = EnsurePreviousLocalToWorld(m_EntityManager, entity);

	const glm::mat4 localM = ComposeLocalMatrix(local->Read());
	localMatrix.value = FromGLM(localM);

	prevLocalToWorld.value = localToWorld.value;

	glm::mat4 worldM(1.0f);
	if (hasParentWorld)
	{
		worldM = ComposeWorldWithInheritance(ToGLM(parentWorld), local->Read(), *hierarchy);
	}
	else
	{
		worldM = localM;
	}

	localToWorld.value = FromGLM(worldM);

	auto child = hierarchy->firstChild;
	while (child.has_value())
	{
		const EZ::Entity currentChild = *child;
		auto* childHierarchy = TryGetHierarchy(m_EntityManager, currentChild);
		auto next = childHierarchy ? childHierarchy->nextSibling : std::optional<EZ::Entity>{};

		UpdateNodeRecursive(currentChild, localToWorld.value, true);
		child = next;
	}
}

void TransformComputeSystem::UpdateWorldTransforms()
{
	m_EntityManager.ForEach<BaseProtocol::LocalTransform, BaseProtocol::TransformHierarchy>(
		[this](EZ::Entity entity, BaseProtocol::LocalTransform& local, BaseProtocol::TransformHierarchy& hierarchy)
		{
			(void)local;

			if (hierarchy.parent.has_value())
			{
				return;
			}

			UpdateNodeRecursive(entity, DataProtocol::Mat4{}, false);
		}
	);
}