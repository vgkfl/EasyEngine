#pragma once
#ifndef __ROOT_MOTION_CONSUME_SCRIPT_H__
#define __ROOT_MOTION_CONSUME_SCRIPT_H__

#include "core/ScriptManager/IScriptBehaviour.h"

class RootMotionConsumeScript final : public EZ::IScriptBehaviour
{
public:
	void Awake() override {}
	void Start() override {}
	void Update() override;
	void FixedUpdate() override {}
	void OnDestroy() override {}

private:
	// 你的角色当前是 100 倍缩放，先用这个倍率做最小测试
	float m_RootMotionScale = 100.0f;

	// 先只同步位置，旋转后面再接
	bool m_IgnoreY = true;
};

#endif