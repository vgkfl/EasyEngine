#pragma once
#ifndef __C_P_CHARACTER_ASSET_MANAGER_H__
#define __C_P_CHARACTER_ASSET_MANAGER_H__

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "BaseProtocol/Animation/AnimatorComponent.h"
#include "DataProtocol/AnimationClipAsset.h"
#include "DataProtocol/AnimatorControllerAsset.h"
#include "DataProtocol/ImageAsset.h"
#include "DataProtocol/MaterialAsset.h"
#include "DataProtocol/MeshAsset.h"
#include "DataProtocol/SkeletonAsset.h"
#include "DataProtocol/SkinAsset.h"
#include "DataProtocol/MathTypes.h"
#include "core/Entity/Entity.h"

namespace EZ
{
	struct WorldContext;
}

namespace ControlProtocol
{
	class CharacterAssetManager
	{
	public:
		struct CharacterAssetEntry
		{
			std::unique_ptr<DataProtocol::MeshAsset> meshAsset;
			std::unique_ptr<DataProtocol::SkeletonAsset> skeletonAsset;
			std::unique_ptr<DataProtocol::SkinAsset> skinAsset;
			std::vector<std::unique_ptr<DataProtocol::MaterialAsset>> materialAssets;
			std::vector<std::unique_ptr<DataProtocol::ImageAsset>> imageAssets;
			std::unordered_map<std::string, std::unique_ptr<DataProtocol::AnimationClipAsset>> clips;

			bool IsRenderable() const
			{
				return meshAsset != nullptr &&
					skeletonAsset != nullptr &&
					skinAsset != nullptr &&
					!skeletonAsset->bones.empty() &&
					!skinAsset->boneBindings.empty();
			}
		};

	public:
		void Clear();

		bool LoadCharacter(
			const std::string& characterName,
			const std::string& modelFbxPath,
			EZ::f32 globalScale = 0.01f,
			EZ::u32 maxBonesPerVertex = 4);

		bool LoadAnimationClip(
			const std::string& characterName,
			const std::string& clipName,
			const std::string& animFbxPath,
			EZ::f32 globalScale = 0.01f,
			EZ::u32 maxBonesPerVertex = 4);

		const CharacterAssetEntry* FindCharacter(const std::string& characterName) const;
		const DataProtocol::AnimationClipAsset* FindClip(const std::string& characterName, const std::string& clipName) const;

		// -----------------------------
		// AnimatorController 资产构建
		// -----------------------------
		bool CreateController(
			const std::string& controllerName,
			const std::string& defaultState);

		bool AddControllerParameter(
			const std::string& controllerName,
			const DataProtocol::AnimatorParameterDesc& parameter);

		bool AddControllerState(
			const std::string& controllerName,
			const std::string& stateName,
			const std::string& characterName,
			const std::string& clipName,
			bool looping = true,
			EZ::f32 speed = 1.0f,
			bool applyScaleKeys = false);

		bool AddControllerTransition(
			const std::string& controllerName,
			const DataProtocol::AnimatorTransitionDesc& transition);

		const DataProtocol::AnimatorControllerAsset* FindController(
			const std::string& controllerName) const;

		EZ::Entity SpawnCharacter(
			EZ::WorldContext& world,
			const std::string& characterName,
			const std::string& controllerName,
			const DataProtocol::Vec3& position,
			const DataProtocol::Vec3& scale,
			const BaseProtocol::AnimatorComponent& animatorTemplate = BaseProtocol::AnimatorComponent{}) const;

	private:
		std::unordered_map<std::string, CharacterAssetEntry> m_Characters;
		std::unordered_map<std::string, std::unique_ptr<DataProtocol::AnimatorControllerAsset>> m_Controllers;
	};
}

#endif