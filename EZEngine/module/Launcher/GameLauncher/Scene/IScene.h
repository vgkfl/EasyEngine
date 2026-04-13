#pragma once
#ifndef __GAME_SCENE_I_SCENE_H__
#define __GAME_SCENE_I_SCENE_H__

#include <vector>

#include "core/Entity/Entity.h"

namespace EZ
{
	struct WorldContext;
}

namespace GameScene
{
	class IScene
	{
	public:
		virtual ~IScene() = default;

		virtual const char* GetSceneId() const = 0;

		// 场景构建时，把顶层根实体都塞进 outSceneRoots，SceneManager 用它做稳定卸载
		virtual void Build(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots) = 0;

		virtual void OnLoaded(EZ::WorldContext& world) { (void)world; }
		virtual void OnUnloading(EZ::WorldContext& world) { (void)world; }
	};
}

#endif