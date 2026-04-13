#include "SceneSmokeTestScript.h"

#include <cmath>
#include <cstdio>
#include <string>

#include "BaseProtocol/Transform/Transform.h"
#include "ControlProtocol/InputController/InputController.h"
#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Input/InputTypes.h"

void SceneSmokeTestScript::Awake()
{
	std::printf("[SceneSmokeTest] Awake entity=%u\n", static_cast<EZ::u32>(GetEntity()));
}

void SceneSmokeTestScript::Start()
{
	if (auto* localTransform = TryGetComponent<BaseProtocol::LocalTransform>())
	{
		m_BaseHeight = localTransform->Read().position.y;
	}

	std::printf("[SceneSmokeTest] Start entity=%u\n", static_cast<EZ::u32>(GetEntity()));
}

void SceneSmokeTestScript::Update()
{
	auto* project = TryGetContext<EZ::ProjectContext>();
	const float deltaTime = project ? project->deltaTime : (1.0f / 60.0f);

	m_Time += deltaTime;
	m_TitleAccum += deltaTime;

	UpdateMovement(deltaTime);
	UpdateJumpAnimation(deltaTime);

	if (m_TitleAccum >= 0.2f)
	{
		m_TitleAccum = 0.0f;
		UpdateWindowTitle();
	}
}

void SceneSmokeTestScript::FixedUpdate()
{
	++m_FixedTicks;
	if ((m_FixedTicks % 120) == 0)
	{
		std::printf("[SceneSmokeTest] FixedUpdate ticks=%d\n", m_FixedTicks);
	}
}

void SceneSmokeTestScript::OnDestroy()
{
	std::printf("[SceneSmokeTest] Destroy entity=%u\n", static_cast<EZ::u32>(GetEntity()));
}

void SceneSmokeTestScript::UpdateMovement(float deltaTime)
{
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

	const float moveSpeed = sprint ? 4.0f : 2.0f;

	transform.position.x += input->GetMoveX() * moveSpeed * deltaTime;
	transform.position.z += input->GetMoveY() * moveSpeed * deltaTime;

	if (input->IsKeyDown(EZ::KeyCode::Q))
	{
		transform.position.y += moveSpeed * deltaTime;
		m_BaseHeight = transform.position.y;
	}

	if (input->IsKeyDown(EZ::KeyCode::E))
	{
		transform.position.y -= moveSpeed * deltaTime;
		m_BaseHeight = transform.position.y;
	}

	// 按空格重新触发“两下跳”
	if (input->IsKeyPressed(EZ::KeyCode::Space))
	{
		m_JumpPhase = 1;
	}
}

void SceneSmokeTestScript::UpdateJumpAnimation(float deltaTime)
{
	auto* localTransform = TryGetComponent<BaseProtocol::LocalTransform>();
	if (!localTransform)
	{
		return;
	}

	auto& transform = localTransform->Get();

	// 启动时自动跳两下，便于观察 Update / 渲染 / 光照
	if (m_Time < 1.5f && m_JumpPhase == 0)
	{
		m_JumpPhase = 1;
	}

	float jumpOffset = 0.0f;

	if (m_JumpPhase == 1)
	{
		jumpOffset = std::sin(m_Time * 6.0f) * 0.6f;
		if (m_Time > 1.2f)
		{
			m_JumpPhase = 2;
		}
	}
	else if (m_JumpPhase == 2)
	{
		jumpOffset = std::sin((m_Time - 1.2f) * 7.0f) * 0.35f;
		if (m_Time > 2.0f)
		{
			m_JumpPhase = 3;
		}
	}
	else
	{
		jumpOffset = 0.0f;
	}

	if (jumpOffset < 0.0f)
	{
		jumpOffset = 0.0f;
	}

	transform.position.y = m_BaseHeight + jumpOffset;

	// 顺手做一点旋转，方便确认 3D 空间里真的在动
	transform.rotation.y += 45.0f * deltaTime;
}

void SceneSmokeTestScript::UpdateWindowTitle()
{
	auto* window = GetPrimaryWindow();
	auto* localTransform = TryGetComponent<BaseProtocol::LocalTransform>();

	if (!window || !localTransform)
	{
		return;
	}

	const auto& t = localTransform->Read();

	char title[256]{};
	std::snprintf(
		title,
		sizeof(title),
		"EZEngine | Cube Pos(%.2f, %.2f, %.2f) RotY=%.2f Time=%.2f",
		t.position.x, t.position.y, t.position.z, t.rotation.y, m_Time);

	window->SetTitle(title);
}