#include "RootMotionConsumeScript.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

#include "BaseProtocol/Animation/AnimatorComponent.h"
#include "BaseProtocol/Transform/Transform.h"
#include "ControlProtocol/EntityManager/EntityManager.h"
#include "ControlProtocol/TransformManager/TransformManager.h"
#include "core/Context/RunTimeContext/WorldContext.h"

namespace
{
	static glm::vec3 ToGlm(const DataProtocol::Vec3& v)
	{
		return glm::vec3(v.x, v.y, v.z);
	}

	static DataProtocol::Vec3 ToDataVec3(const glm::vec3& v)
	{
		return DataProtocol::Vec3{ v.x, v.y, v.z };
	}

	static glm::quat ToGlm(const DataProtocol::Quat& q)
	{
		return glm::quat(q.w, q.x, q.y, q.z);
	}
}

void RootMotionConsumeScript::Update()
{
	auto* world = GetWorld();
	if (!world)
	{
		return;
	}

	auto* entityManager = world->TryGet<ControlProtocol::EntityManager>();
	auto* transformManager = world->TryGet<ControlProtocol::TransformManager>();
	if (!entityManager || !transformManager)
	{
		return;
	}

	auto* animator = entityManager->TryGetComponent<BaseProtocol::AnimatorComponent>(GetEntity());
	auto* localTransform = entityManager->TryGetComponent<BaseProtocol::LocalTransform>(GetEntity());
	if (!animator || !localTransform)
	{
		return;
	}

	if (!animator->applyRootMotion || !animator->hasRootMotion)
	{
		return;
	}

	auto& t = localTransform->Get();

	// 动画提取出来的是角色局部空间位移
	glm::vec3 deltaLocal = ToGlm(animator->extractedRootMotion.deltaPosition) * m_RootMotionScale;

	if (m_IgnoreY)
	{
		deltaLocal.y = 0.0f;
	}

	// 用当前实体朝向把局部位移转到世界/父空间方向
	const glm::quat rotation = ToGlm(t.rotation);
	const glm::vec3 deltaMoved = rotation * deltaLocal;

	t.position = ToDataVec3(ToGlm(t.position) + deltaMoved);

	transformManager->MarkWorldDirty(GetEntity());
}