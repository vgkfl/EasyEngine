#pragma once
#ifndef __TEST01_PERF_SCENES_H__
#define __TEST01_PERF_SCENES_H__

#include "Launcher/GameLauncher/Scene/IScene.h"

namespace Test01Scenes
{
	class BaselineScene final : public GameScene::IScene
	{
	public:
		const char* GetSceneId() const override { return "Baseline"; }
		void Build(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots) override;
	};

	class CrowdSyncIdleScene final : public GameScene::IScene
	{
	public:
		const char* GetSceneId() const override { return "CrowdSyncIdle"; }
		void Build(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots) override;
	};

	class CrowdIdlePhaseOffsetScene final : public GameScene::IScene
	{
	public:
		const char* GetSceneId() const override { return "CrowdIdlePhaseOffset"; }
		void Build(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots) override;
	};

	class CrowdMixedStatesScene final : public GameScene::IScene
	{
	public:
		const char* GetSceneId() const override { return "CrowdMixedStates"; }
		void Build(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots) override;
	};

	class RootMotionRunwayScene final : public GameScene::IScene
	{
	public:
		const char* GetSceneId() const override { return "RootMotionRunway"; }
		void Build(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots) override;
	};

	class ThirdPersonControllerScene final : public GameScene::IScene
	{
	public:
		const char* GetSceneId() const override { return "ThirdPersonController"; }
		void Build(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots) override;
	};
}

#endif