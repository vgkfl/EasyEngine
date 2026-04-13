#include "ThirdPersonControllerScript.h"

#include <cmath>
#include <algorithm>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

#include "BaseProtocol/Animation/AnimatorComponent.h"
#include "BaseProtocol/Camera/CameraComponent.h"
#include "BaseProtocol/Transform/Transform.h"

#include "ControlProtocol/EntityManager/EntityManager.h"
#include "ControlProtocol/InputController/InputController.h"
#include "ControlProtocol/TransformManager/TransformManager.h"

#include "core/Context/RunTimeContext/WorldContext.h"
#include "core/Input/InputTypes.h"

namespace
{
	static constexpr float kPi = 3.14159265358979323846f;

	static float DegToRad(float deg)
	{
		return deg * (kPi / 180.0f);
	}

	static float RadToDeg(float rad)
	{
		return rad * (180.0f / kPi);
	}

	static float NormalizeDegrees(float deg)
	{
		while (deg > 180.0f) deg -= 360.0f;
		while (deg < -180.0f) deg += 360.0f;
		return deg;
	}

	static float LerpAngleDegrees(float current, float target, float t)
	{
		float delta = NormalizeDegrees(target - current);
		return current + delta * t;
	}

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

	static DataProtocol::Quat ToDataQuat(const glm::quat& q)
	{
		glm::quat n = glm::normalize(q);
		return DataProtocol::Quat{ n.x, n.y, n.z, n.w };
	}
}

void ThirdPersonControllerScript::Awake()
{
	ResolveReferences();
}

void ThirdPersonControllerScript::Start()
{
	ResolveReferences();
}

bool ThirdPersonControllerScript::ResolveReferences()
{
	auto* world = GetWorld();
	if (!world)
	{
		return false;
	}

	auto* entityManager = world->TryGet<ControlProtocol::EntityManager>();
	auto* transformManager = world->TryGet<ControlProtocol::TransformManager>();
	if (!entityManager || !transformManager)
	{
		return false;
	}

	m_VisualEntity = EZ::Entity{};
	m_CameraEntity = EZ::Entity{};

	auto child = transformManager->GetFirstChild(GetEntity());
	while (child.has_value())
	{
		const EZ::Entity e = *child;

		if (entityManager->HasComponent<BaseProtocol::AnimatorComponent>(e))
		{
			m_VisualEntity = e;
		}

		if (entityManager->HasComponent<BaseProtocol::CameraComponent>(e))
		{
			m_CameraEntity = e;
		}

		child = transformManager->GetNextSibling(e);
	}

	return m_VisualEntity != EZ::Entity{} && m_CameraEntity != EZ::Entity{};
}

void ThirdPersonControllerScript::Update()
{
	if (!ResolveReferences())
	{
		return;
	}

	UpdateCameraOrbit();
	UpdateMovement();
}

void ThirdPersonControllerScript::UpdateCameraOrbit()
{
	auto* world = GetWorld();
	if (!world)
	{
		return;
	}

	auto* entityManager = world->TryGet<ControlProtocol::EntityManager>();
	auto* input = world->TryGet<ControlProtocol::InputController>();
	if (!entityManager || !input)
	{
		return;
	}

	auto* cameraLocal = entityManager->TryGetComponent<BaseProtocol::LocalTransform>(m_CameraEntity);
	if (!cameraLocal)
	{
		return;
	}

	m_CameraYaw += input->GetMouseDeltaX() * m_MouseSensitivity;
	m_CameraPitch -= input->GetMouseDeltaY() * m_MouseSensitivity;
	m_CameraPitch = std::clamp(m_CameraPitch, m_MinPitch, m_MaxPitch);

	const float yawRad = DegToRad(m_CameraYaw);
	const float pitchRad = DegToRad(m_CameraPitch);

	const float cosPitch = std::cos(pitchRad);
	const float sinPitch = std::sin(pitchRad);
	const float sinYaw = std::sin(yawRad);
	const float cosYaw = std::cos(yawRad);

	glm::vec3 targetLocal(0.0f, m_CameraTargetHeight, 0.0f);
	glm::vec3 cameraOffset(
		sinYaw * cosPitch * m_CameraDistance,
		sinPitch * m_CameraDistance + m_CameraTargetHeight,
		-cosYaw * cosPitch * m_CameraDistance);

	auto& t = cameraLocal->Get();
	t.position = ToDataVec3(cameraOffset);

	const glm::vec3 forward = glm::normalize(targetLocal - cameraOffset);
	const glm::mat4 view = glm::lookAt(cameraOffset, targetLocal, glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::quat camRot = glm::conjugate(glm::quat_cast(view));
	t.rotation = ToDataQuat(camRot);
}

void ThirdPersonControllerScript::UpdateMovement()
{
	auto* world = GetWorld();
	if (!world)
	{
		return;
	}

	auto* entityManager = world->TryGet<ControlProtocol::EntityManager>();
	auto* transformManager = world->TryGet<ControlProtocol::TransformManager>();
	auto* input = world->TryGet<ControlProtocol::InputController>();
	if (!entityManager || !transformManager || !input)
	{
		return;
	}

	auto* rootLocal = entityManager->TryGetComponent<BaseProtocol::LocalTransform>(GetEntity());
	auto* visualLocal = entityManager->TryGetComponent<BaseProtocol::LocalTransform>(m_VisualEntity);
	auto* animator = entityManager->TryGetComponent<BaseProtocol::AnimatorComponent>(m_VisualEntity);
	if (!rootLocal || !visualLocal || !animator)
	{
		return;
	}

	const bool forward = input->IsKeyDown(EZ::KeyCode::W);
	const bool backward = input->IsKeyDown(EZ::KeyCode::S);
	const bool left = input->IsKeyDown(EZ::KeyCode::A);
	const bool right = input->IsKeyDown(EZ::KeyCode::D);

	const bool shift =
		input->IsKeyDown(EZ::KeyCode::LeftShift) ||
		input->IsKeyDown(EZ::KeyCode::RightShift);

	float inputX = 0.0f;
	float inputY = 0.0f;

	if (left)  inputX -= 1.0f;
	if (right) inputX += 1.0f;
	if (forward) inputY += 1.0f;
	if (backward) inputY -= 1.0f;

	glm::vec2 moveInput(inputX, inputY);
	const float inputLen = glm::length(moveInput);
	const bool hasMove = inputLen > 0.001f;

	if (!hasMove)
	{
		animator->SetFloat("Speed", 0.0f);
		return;
	}

	moveInput /= inputLen;

	auto* cameraLocal = entityManager->TryGetComponent<BaseProtocol::LocalTransform>(m_CameraEntity);
	if (!cameraLocal)
	{
		animator->SetFloat("Speed", 0.0f);
		return;
	}

	// 直接根据“相机当前位置 -> 角色观察点”计算真实前方向
	const glm::vec3 cameraPosLocal = ToGlm(cameraLocal->Read().position);
	const glm::vec3 lookTargetLocal(0.0f, m_CameraTargetHeight, 0.0f);

	glm::vec3 cameraForward = lookTargetLocal - cameraPosLocal;
	cameraForward.y = 0.0f;

	if (glm::length(cameraForward) <= 0.0001f)
	{
		animator->SetFloat("Speed", 0.0f);
		return;
	}

	cameraForward = glm::normalize(cameraForward);

	// 右方向：up x forward
	glm::vec3 cameraRight = -glm::normalize(
		glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cameraForward));

	// 完全按摄像机前后左右来移动
	glm::vec3 desiredMove =
		cameraForward * moveInput.y +
		cameraRight * moveInput.x;

	if (glm::length(desiredMove) <= 0.0001f)
	{
		animator->SetFloat("Speed", 0.0f);
		return;
	}

	desiredMove = glm::normalize(desiredMove);

	if (glm::length(desiredMove) <= 0.0001f)
	{
		animator->SetFloat("Speed", 0.0f);
		return;
	}

	desiredMove = glm::normalize(desiredMove);
	if (glm::length(desiredMove) <= 0.0001f)
	{
		animator->SetFloat("Speed", 0.0f);
		return;
	}

	desiredMove = glm::normalize(desiredMove);

	const float desiredYaw = RadToDeg(std::atan2(desiredMove.x, desiredMove.z));
	m_VisualYawDegrees = LerpAngleDegrees(
		m_VisualYawDegrees,
		desiredYaw,
		std::clamp(m_RotateLerpSpeed * 0.016f, 0.0f, 1.0f));

	{
		auto& visualT = visualLocal->Get();
		const glm::quat visualRot = glm::angleAxis(
			DegToRad(m_VisualYawDegrees),
			glm::vec3(0.0f, 1.0f, 0.0f));
		visualT.rotation = ToDataQuat(visualRot);
		transformManager->MarkWorldDirty(m_VisualEntity);
	}

	if (shift)
	{
		animator->SetFloat("Speed", 1.0f);
	}
	else
	{
		animator->SetFloat("Speed", 0.5f);
	}

	if (animator->applyRootMotion && animator->hasRootMotion)
	{
		glm::vec3 deltaLocal = ToGlm(animator->extractedRootMotion.deltaPosition) * m_RootMotionScale;
		deltaLocal.y = 0.0f;

		const glm::quat visualRot = ToGlm(visualLocal->Read().rotation);
		const glm::vec3 deltaWorld = visualRot * deltaLocal;

		auto& rootT = rootLocal->Get();
		rootT.position = ToDataVec3(ToGlm(rootT.position) + deltaWorld);
		transformManager->MarkWorldDirty(GetEntity());
	}
}