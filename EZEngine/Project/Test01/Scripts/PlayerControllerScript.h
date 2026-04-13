#pragma once
#ifndef __PLAYER_CONTROLLER_SCRIPT_H__
#define __PLAYER_CONTROLLER_SCRIPT_H__

#include "core/ScriptManager/IScriptBehaviour.h"

class PlayerControllerScript final : public EZ::IScriptBehaviour
{
public:
	void Awake() override;
	void Start() override;
	void Update() override;
	void FixedUpdate() override;
	void OnDestroy() override;

private:
	float m_ElapsedTime = 0.0f;
	float m_TitleRefreshAccumulator = 0.0f;
	int m_FixedTickCount = 0;
	int m_FrameCounter = 0;

	float m_Yaw = 0.0f;
	float m_Pitch = 0.0f;
};

#endif