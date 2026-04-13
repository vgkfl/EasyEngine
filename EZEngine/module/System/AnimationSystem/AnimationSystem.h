#pragma once
#ifndef __ANIMATION_SYSTEM_H__
#define __ANIMATION_SYSTEM_H__

#include "core/System/ISystem.h"



class AnimationSystem : public EZ::ISystem
{
public:
	const char* GetName() const override { return "AnimationSystem"; }

	// 这一阶段先不需要专门初始化逻辑，直接给空实现，避免后面链接错误
	int Initialize(EZ::ProjectContext& project, EZ::WorldContext& world) override
	{
		(void)project;
		(void)world;
		return 0;
	}

	void Shutdown(EZ::ProjectContext& project, EZ::WorldContext& world) override
	{
		(void)project;
		(void)world;
	}

	// 这里必须和 cpp 里的实现一致
	void Update(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime) override;
};

#endif