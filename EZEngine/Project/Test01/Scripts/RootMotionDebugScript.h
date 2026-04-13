#pragma once
#ifndef __ROOT_MOTION_DEBUG_SCRIPT_H__
#define __ROOT_MOTION_DEBUG_SCRIPT_H__

#include "core/ScriptManager/IScriptBehaviour.h"

class RootMotionDebugScript final : public EZ::IScriptBehaviour
{
public:
	void Awake() override;
	void Start() override;
	void Update() override;
	void FixedUpdate() override {}
	void OnDestroy() override {}

private:
	int m_FrameCounter = 0;
};

#endif