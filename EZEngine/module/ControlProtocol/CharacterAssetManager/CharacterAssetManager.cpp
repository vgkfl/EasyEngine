#include "ControlProtocol/CharacterAssetManager/CharacterAssetManager.h"

#include <cstdio>
#include <utility>

#include "BaseProtocol/Animation/BonePoseComponent.h"
#include "BaseProtocol/Animation/SkinningPaletteComponent.h"
#include "BaseProtocol/Mesh/SkinnedMeshRendererComponent.h"
#include "BaseProtocol/Transform/Transform.h"
#include "ControlProtocol/EntityManager/EntityManager.h"
#include "ControlProtocol/TransformManager/TransformManager.h"
#include "Tool/FBXImporter/FBXImporter.h"
#include "core/Context/RunTimeContext/WorldContext.h"

namespace ControlProtocol
{
	void CharacterAssetManager::Clear()
	{
		m_Characters.clear();
		m_Controllers.clear();
	}

	bool CharacterAssetManager::LoadCharacter(
		const std::string& characterName,
		const std::string& modelFbxPath,
		EZ::f32 globalScale,
		EZ::u32 maxBonesPerVertex)
	{
		Tool::FBXImportOptions options{};
		options.importAnimations = false;
		options.globalScale = globalScale;
		options.maxBonesPerVertex = maxBonesPerVertex;

		auto importResult = Tool::FBXImporter::Import(modelFbxPath, options);

		auto& entry = m_Characters[characterName];
		entry.meshAsset = std::move(importResult.meshAsset);
		entry.skeletonAsset = std::move(importResult.skeletonAsset);
		entry.skinAsset = std::move(importResult.skinAsset);
		entry.materialAssets = std::move(importResult.materialAssets);
		entry.imageAssets = std::move(importResult.imageAssets);

		const bool ok = entry.IsRenderable();
		std::printf(
			"[CharacterAssetManager] LoadCharacter '%s' => mesh=%p bones=%zu bindings=%zu\n",
			characterName.c_str(),
			(void*)entry.meshAsset.get(),
			entry.skeletonAsset ? entry.skeletonAsset->bones.size() : 0,
			entry.skinAsset ? entry.skinAsset->boneBindings.size() : 0);
		std::printf(
			"[CharacterAssetManager] subMeshes=%zu\n",
			entry.meshAsset ? entry.meshAsset->subMeshes.size() : 0);
		return ok;
	}

	bool CharacterAssetManager::LoadAnimationClip(
		const std::string& characterName,
		const std::string& clipName,
		const std::string& animFbxPath,
		EZ::f32 globalScale,
		EZ::u32 maxBonesPerVertex)
	{
		auto entryIt = m_Characters.find(characterName);
		if (entryIt == m_Characters.end())
		{
			return false;
		}

		Tool::FBXImportOptions options{};
		options.importMesh = false;
		options.importSkin = false;
		options.importMaterials = false;
		options.importImages = false;
		options.importAnimations = true;
		options.importSkeleton = true;
		options.globalScale = globalScale;
		options.maxBonesPerVertex = maxBonesPerVertex;

		auto importResult = Tool::FBXImporter::Import(animFbxPath, options);
		if (importResult.animationAssets.empty() || !importResult.animationAssets.front())
		{
			return false;
		}

		auto& entry = entryIt->second;
		entry.clips[clipName] = std::move(importResult.animationAssets.front());
		std::printf(
			"[CharacterAssetManager] LoadAnimation '%s/%s' => tracks=%zu\n",
			characterName.c_str(),
			clipName.c_str(),
			entry.clips[clipName] ? entry.clips[clipName]->tracks.size() : 0);
		return true;
	}

	const CharacterAssetManager::CharacterAssetEntry* CharacterAssetManager::FindCharacter(const std::string& characterName) const
	{
		auto it = m_Characters.find(characterName);
		if (it == m_Characters.end())
		{
			return nullptr;
		}
		return &it->second;
	}

	const DataProtocol::AnimationClipAsset* CharacterAssetManager::FindClip(const std::string& characterName, const std::string& clipName) const
	{
		const CharacterAssetEntry* entry = FindCharacter(characterName);
		if (!entry)
		{
			return nullptr;
		}

		auto clipIt = entry->clips.find(clipName);
		if (clipIt == entry->clips.end())
		{
			return nullptr;
		}
		return clipIt->second.get();
	}

	bool CharacterAssetManager::CreateController(
		const std::string& controllerName,
		const std::string& defaultState)
	{
		auto& controller = m_Controllers[controllerName];
		if (!controller)
		{
			controller = std::make_unique<DataProtocol::AnimatorControllerAsset>();
		}

		controller->controllerName = controllerName;
		controller->defaultState = defaultState;
		return true;
	}

	bool CharacterAssetManager::AddControllerParameter(
		const std::string& controllerName,
		const DataProtocol::AnimatorParameterDesc& parameter)
	{
		auto it = m_Controllers.find(controllerName);
		if (it == m_Controllers.end() || !it->second)
		{
			return false;
		}

		it->second->parameters.push_back(parameter);
		return true;
	}

	bool CharacterAssetManager::AddControllerState(
		const std::string& controllerName,
		const std::string& stateName,
		const std::string& characterName,
		const std::string& clipName,
		bool looping,
		EZ::f32 speed,
		bool applyScaleKeys)
	{
		auto controllerIt = m_Controllers.find(controllerName);
		if (controllerIt == m_Controllers.end() || !controllerIt->second)
		{
			return false;
		}

		const DataProtocol::AnimationClipAsset* clip = FindClip(characterName, clipName);
		if (!clip)
		{
			return false;
		}

		DataProtocol::AnimatorStateDesc state{};
		state.name = stateName;
		state.clipName = clipName;
		state.clipAsset = clip;
		state.looping = looping;
		state.speed = speed;
		state.applyScaleKeys = applyScaleKeys;

		controllerIt->second->states.push_back(state);
		return true;
	}

	bool CharacterAssetManager::AddControllerTransition(
		const std::string& controllerName,
		const DataProtocol::AnimatorTransitionDesc& transition)
	{
		auto controllerIt = m_Controllers.find(controllerName);
		if (controllerIt == m_Controllers.end() || !controllerIt->second)
		{
			return false;
		}

		controllerIt->second->transitions.push_back(transition);
		return true;
	}

	const DataProtocol::AnimatorControllerAsset* CharacterAssetManager::FindController(
		const std::string& controllerName) const
	{
		auto it = m_Controllers.find(controllerName);
		if (it == m_Controllers.end())
		{
			return nullptr;
		}
		return it->second.get();
	}

	EZ::Entity CharacterAssetManager::SpawnCharacter(
		EZ::WorldContext& world,
		const std::string& characterName,
		const std::string& controllerName,
		const DataProtocol::Vec3& position,
		const DataProtocol::Vec3& scale,
		const BaseProtocol::AnimatorComponent& animatorTemplate) const
	{
		auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
		auto* transformManager = world.TryGet<ControlProtocol::TransformManager>();
		if (!entityManager || !transformManager)
		{
			return EZ::Entity{};
		}

		const CharacterAssetEntry* entry = FindCharacter(characterName);
		if (!entry || !entry->IsRenderable())
		{
			return EZ::Entity{};
		}

		EZ::Entity entity = transformManager->CreateEntity(position);

		auto& localTransform = entityManager->GetComponent<BaseProtocol::LocalTransform>(entity);
		localTransform.Get().scale = scale;

		auto& renderer = entityManager->AddComponent<BaseProtocol::SkinnedMeshRendererComponent>(entity);
		renderer.meshAsset = entry->meshAsset.get();
		renderer.skeletonAsset = entry->skeletonAsset.get();
		renderer.skinAsset = entry->skinAsset.get();
		renderer.visible = true;
		renderer.castShadow = true;
		renderer.receiveShadow = true;
		renderer.materialAssets.reserve(entry->materialAssets.size());
		for (const auto& material : entry->materialAssets)
		{
			renderer.materialAssets.push_back(material.get());
		}

		auto& animator = entityManager->AddComponent<BaseProtocol::AnimatorComponent>(entity);
		animator = animatorTemplate;
		animator.BindController(FindController(controllerName));

		entityManager->AddComponent<BaseProtocol::BonePoseComponent>(entity);
		entityManager->AddComponent<BaseProtocol::SkinningPaletteComponent>(entity);

		return entity;
	}
}