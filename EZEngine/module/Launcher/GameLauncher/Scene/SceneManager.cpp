#include "Launcher/GameLauncher/Scene/SceneManager.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "Launcher/GameLauncher/Scene/IScene.h"

#include "ControlProtocol/EntityManager/EntityManager.h"
#include "ControlProtocol/TransformManager/TransformManager.h"
#include "core/Context/RunTimeContext/WorldContext.h"

namespace GameScene
{
	bool SceneManager::RegisterScene(std::unique_ptr<IScene> scene)
	{
		if (!scene)
		{
			return false;
		}

		const char* sceneId = scene->GetSceneId();
		if (!sceneId || sceneId[0] == '\0')
		{
			return false;
		}

		auto [it, inserted] = m_Scenes.emplace(sceneId, std::move(scene));
		return inserted;
	}

	void SceneManager::GetRegisteredSceneIds(std::vector<std::string>& outSceneIds) const
	{
		outSceneIds.clear();
		outSceneIds.reserve(m_Scenes.size());

		for (const auto& [sceneId, scene] : m_Scenes)
		{
			(void)scene;
			outSceneIds.push_back(sceneId);
		}

		std::sort(outSceneIds.begin(), outSceneIds.end());
	}

	IScene* SceneManager::FindScene(const std::string& sceneId)
	{
		auto it = m_Scenes.find(sceneId);
		return (it != m_Scenes.end()) ? it->second.get() : nullptr;
	}

	const IScene* SceneManager::FindScene(const std::string& sceneId) const
	{
		auto it = m_Scenes.find(sceneId);
		return (it != m_Scenes.end()) ? it->second.get() : nullptr;
	}

	bool SceneManager::LoadScene(const std::string& sceneId, EZ::WorldContext& world)
	{
		IScene* target = FindScene(sceneId);
		if (!target)
		{
			return false;
		}

		UnloadCurrentScene(world);

		std::vector<EZ::Entity> newRoots;
		target->Build(world, newRoots);

		m_CurrentSceneRoots = std::move(newRoots);
		m_CurrentSceneId = sceneId;

		target->OnLoaded(world);
		return true;
	}

	bool SceneManager::ReloadCurrentScene(EZ::WorldContext& world)
	{
		if (m_CurrentSceneId.empty())
		{
			return false;
		}
		return LoadScene(m_CurrentSceneId, world);
	}

	void SceneManager::UnloadCurrentScene(EZ::WorldContext& world)
	{
		if (!m_CurrentSceneId.empty())
		{
			if (IScene* current = FindScene(m_CurrentSceneId))
			{
				current->OnUnloading(world);
			}
		}

		DestroyTrackedRoots(world);
		m_CurrentSceneId.clear();
	}

	void SceneManager::DestroyTrackedRoots(EZ::WorldContext& world)
	{
		auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
		auto* transformManager = world.TryGet<ControlProtocol::TransformManager>();
		if (!entityManager || !transformManager)
		{
			m_CurrentSceneRoots.clear();
			return;
		}

		std::vector<EZ::Entity> destroyList;
		std::vector<EZ::Entity> subtree;

		for (EZ::Entity root : m_CurrentSceneRoots)
		{
			if (!entityManager->IsValid(root))
			{
				continue;
			}

			subtree.clear();

			if (transformManager->HasNode(root))
			{
				transformManager->BuildSubtreePreorder(root, subtree);
			}
			else
			{
				subtree.push_back(root);
			}

			destroyList.insert(destroyList.end(), subtree.begin(), subtree.end());
		}

		// 去重，防御性处理
		std::sort(
			destroyList.begin(),
			destroyList.end(),
			[](EZ::Entity a, EZ::Entity b)
			{
				return static_cast<EZ::u32>(a) < static_cast<EZ::u32>(b);
			});
		destroyList.erase(std::unique(destroyList.begin(), destroyList.end()), destroyList.end());

		for (auto it = destroyList.rbegin(); it != destroyList.rend(); ++it)
		{
			const EZ::Entity entity = *it;
			if (!entityManager->IsValid(entity))
			{
				continue;
			}

			if (transformManager->HasNode(entity))
			{
				transformManager->UnregisterEntity(entity);
			}

			entityManager->DestroyEntity(entity);
		}

		transformManager->GarbageCollectInvalidEntities();
		m_CurrentSceneRoots.clear();
	}
}