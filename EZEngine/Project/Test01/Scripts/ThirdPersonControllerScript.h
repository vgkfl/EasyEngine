#pragma once
#ifndef __THIRD_PERSON_CONTROLLER_SCRIPT_H__
#define __THIRD_PERSON_CONTROLLER_SCRIPT_H__

#include "core/ScriptManager/IScriptBehaviour.h"
#include "core/Entity/Entity.h"

class ThirdPersonControllerScript final : public EZ::IScriptBehaviour
{
public:
	void Awake() override;
	void Start() override;
	void Update() override;
	void FixedUpdate() override {}
	void OnDestroy() override {}

private:
	bool ResolveReferences();
	void UpdateCameraOrbit();
	void UpdateMovement();

private:
	EZ::Entity m_VisualEntity = EZ::Entity{};
	EZ::Entity m_CameraEntity = EZ::Entity{};

private:
	float m_RootMotionScale = 100.0f;

	float m_VisualYawDegrees = 0.0f;
	float m_TurnDamping = 0.14f;     // 越大越快
	float m_SmoothedSpeed = 0.0f;
	float m_SpeedDamping = 0.18f;    // 越大越快
	float m_MoveDeadZone = 0.08f;

	float m_RotateLerpSpeed = 12.0f;

	float m_MouseSensitivity = 0.18f;
	float m_CameraDistance = 5.5f;
	float m_CameraTargetHeight = 1.8f;
	float m_CameraYaw = 0.0f;
	float m_CameraPitch = 15.0f;
	float m_MinPitch = -20.0f;
	float m_MaxPitch = 65.0f;
};

#endif