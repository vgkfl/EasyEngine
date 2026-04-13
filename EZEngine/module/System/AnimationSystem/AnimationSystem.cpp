#include "AnimationSystem.h"

#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Context/RunTimeContext/WorldContext.h"

void AnimationSystem::Update(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime)
{
	(void)project;
	(void)world;
	(void)deltaTime;

	// 第二步开始，这里不再承担任何动画逻辑。
	// 真正的状态机驱动、时间推进、clip 选择、骨骼求值，
	// 都由 AnimatorSystem 完成。
}