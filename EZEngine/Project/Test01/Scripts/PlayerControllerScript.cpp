#include "PlayerControllerScript.h"

#include <cstdio>
#include <string>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "BaseProtocol/Transform/Transform.h"
#include "ControlProtocol/InputController/InputController.h"
#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Input/InputTypes.h"

namespace
{
	glm::quat ToGLMQuat(const DataProtocol::Quat& q)
	{
		return glm::quat(q.w, q.x, q.y, q.z);
	}

	DataProtocol::Quat ToDataQuat(const glm::quat& q)
	{
		DataProtocol::Quat result;
		result.x = q.x;
		result.y = q.y;
		result.z = q.z;
		result.w = q.w;
		return result;
	}
}

void PlayerControllerScript::Awake()
{
	std::printf("[PlayerController] Awake entity=%u\n", static_cast<EZ::u32>(GetEntity()));
}

void PlayerControllerScript::Start()
{
	std::printf("[PlayerController] Start entity=%u\n", static_cast<EZ::u32>(GetEntity()));

	if (auto* localTransform = TryGetComponent<BaseProtocol::LocalTransform>())
	{
		const auto& transform = localTransform->Read();
		const glm::quat rotation = ToGLMQuat(transform.rotation);
		const glm::vec3 euler = glm::eulerAngles(rotation);

		m_Pitch = euler.x;
		m_Yaw = euler.y;
	}
}

void PlayerControllerScript::Update()
{
	auto* project = TryGetContext<EZ::ProjectContext>();
	const float deltaTime = project ? project->deltaTime : (1.0f / 60.0f);
	m_ElapsedTime += deltaTime;
	m_TitleRefreshAccumulator += deltaTime;
	m_FrameCounter += 1;

	auto* input = TryGetContext<ControlProtocol::InputController>();
	auto* localTransform = TryGetComponent<BaseProtocol::LocalTransform>();
	if (!input || !localTransform)
	{
		return;
	}

	auto& transform = localTransform->Get();

	const bool sprint =
		input->IsKeyDown(EZ::KeyCode::LeftShift) ||
		input->IsKeyDown(EZ::KeyCode::RightShift);

	const float moveSpeed = sprint ? 4.5f : 2.2f;
	const float mouseSensitivity = 0.0035f;

	if (input->IsMouseButtonDown(EZ::MouseButton::Right))
	{
		m_Yaw -= input->GetMouseDeltaX() * mouseSensitivity;
		m_Pitch -= input->GetMouseDeltaY() * mouseSensitivity;
	}
	const float maxPitch = glm::radians(85.0f);
	m_Pitch = std::clamp(m_Pitch, -maxPitch, maxPitch);

	const glm::quat yawRotation = glm::angleAxis(m_Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::quat pitchRotation = glm::angleAxis(m_Pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	const glm::quat finalRotation = glm::normalize(yawRotation * pitchRotation);

	transform.rotation = ToDataQuat(finalRotation);

	const glm::vec3 forward = glm::normalize(finalRotation * glm::vec3(0.0f, 0.0f, -1.0f));
	const glm::vec3 right = glm::normalize(finalRotation * glm::vec3(1.0f, 0.0f, 0.0f));

	glm::vec3 move(0.0f);

	if (input->IsKeyDown(EZ::KeyCode::W))
	{
		move += forward;
	}
	if (input->IsKeyDown(EZ::KeyCode::S))
	{
		move -= forward;
	}
	if (input->IsKeyDown(EZ::KeyCode::D))
	{
		move += right;
	}
	if (input->IsKeyDown(EZ::KeyCode::A))
	{
		move -= right;
	}
	if (input->IsKeyDown(EZ::KeyCode::E))
	{
		move.y += 1.0f;
	}
	if (input->IsKeyDown(EZ::KeyCode::Q))
	{
		move.y -= 1.0f;
	}

	const float moveLength = glm::length(move);
	if (moveLength > 0.0001f)
	{
		move /= moveLength;
		transform.position.x += move.x * moveSpeed * deltaTime;
		transform.position.y += move.y * moveSpeed * deltaTime;
		transform.position.z += move.z * moveSpeed * deltaTime;
	}

	if (m_TitleRefreshAccumulator >= 0.2f)
	{
		const float sampleTime = m_TitleRefreshAccumulator;
		const int sampleFrames = m_FrameCounter;

		m_TitleRefreshAccumulator = 0.0f;
		m_FrameCounter = 0;

		const float fps = (sampleTime > 0.0001f)
			? (static_cast<float>(sampleFrames) / sampleTime)
			: 0.0f;

		if (auto* window = GetPrimaryWindow())
		{
			const auto& t = localTransform->Read();
			char title[256]{};
			std::snprintf(
				title,
				sizeof(title),
				"EZEngine | FPS: %.1f | Pos(%.2f, %.2f, %.2f) | WASD Local Move  Q/E UpDown  Mouse Look",
				fps,
				t.position.x, t.position.y, t.position.z);
			window->SetTitle(title);
		}
	}
}

void PlayerControllerScript::FixedUpdate()
{
	++m_FixedTickCount;
	if ((m_FixedTickCount % 120) == 0)
	{
		//std::printf("[PlayerController] FixedUpdate ticks=%d\n", m_FixedTickCount);
	}
}

void PlayerControllerScript::OnDestroy()
{
	//std::printf("[PlayerController] Destroy entity=%u\n", static_cast<EZ::u32>(GetEntity()));
}