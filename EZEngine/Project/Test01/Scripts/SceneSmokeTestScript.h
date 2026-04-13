#pragma once
#ifndef __SCENE_SMOKE_TEST_SCRIPT_H__
#define __SCENE_SMOKE_TEST_SCRIPT_H__

#include "core/ScriptManager/IScriptBehaviour.h"

class SceneSmokeTestScript final : public EZ::IScriptBehaviour
{
public:
	void Awake() override;
	void Start() override;
	void Update() override;
	void FixedUpdate() override;
	void OnDestroy() override;

private:
	void UpdateMovement(float deltaTime);
	void UpdateJumpAnimation(float deltaTime);
	void UpdateWindowTitle();

private:
	float m_Time = 0.0f;
	float m_TitleAccum = 0.0f;
	float m_BaseHeight = 0.0f;
	int m_JumpPhase = 0;
	int m_FixedTicks = 0;
};

#endif