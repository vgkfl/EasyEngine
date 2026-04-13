#pragma once
#ifndef __GAME_SCENE_MANAGER_H__
#define __GAME_SCENE_MANAGER_H__

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/Entity/Entity.h"
#include "Launcher/GameLauncher/Scene/IScene.h"

namespace EZ
{
	struct WorldContext;
}

namespace GameScene
{
	class SceneManager
	{
	public:
		bool RegisterScene(std::unique_ptr<IScene> scene);

		template<typename TScene, typename... Args>
		bool RegisterSceneType(Args&&... args)
		{
			return RegisterScene(std::make_unique<TScene>(std::forward<Args>(args)...));
		}

		bool LoadScene(const std::string& sceneId, EZ::WorldContext& world);
		bool ReloadCurrentScene(EZ::WorldContext& world);

		bool HasCurrentScene() const { return !m_CurrentSceneId.empty(); }
		const std::string& GetCurrentSceneId() const { return m_CurrentSceneId; }

		void GetRegisteredSceneIds(std::vector<std::string>& outSceneIds) const;

		IScene* FindScene(const std::string& sceneId);
		const IScene* FindScene(const std::string& sceneId) const;

		void ClearRegisteredScenes()
		{
			m_CurrentSceneId.clear();
			m_CurrentSceneRoots.clear();
			m_Scenes.clear();
		}

	private:
		void UnloadCurrentScene(EZ::WorldContext& world);
		void DestroyTrackedRoots(EZ::WorldContext& world);

	private:
		std::unordered_map<std::string, std::unique_ptr<IScene>> m_Scenes;
		std::string m_CurrentSceneId;
		std::vector<EZ::Entity> m_CurrentSceneRoots;
	};
}

#endif