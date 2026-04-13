#pragma once
#ifndef __ANIMATOR_SYSTEM_H__
#define __ANIMATOR_SYSTEM_H__

#include <unordered_map>
#include <vector>

#include "core/System/ISystem.h"
#include "core/Types.h"
#include "DataProtocol/AnimationClipAsset.h"
#include "DataProtocol/MathTypes.h"
#include "DataProtocol/SkeletonAsset.h"
#include "DataProtocol/SkinAsset.h"

class AnimatorSystem : public EZ::ISystem
{
public:
	const char* GetName() const override { return "AnimatorSystem"; }

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
		m_SharedPaletteCache.clear();
	}

	void Update(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime) override;

private:
	struct SharedPaletteKey
	{
		const DataProtocol::SkeletonAsset* skeletonAsset = nullptr;
		const DataProtocol::SkinAsset* skinAsset = nullptr;
		const DataProtocol::AnimationClipAsset* clipAsset = nullptr;
		bool applyScaleKeys = false;
		EZ::u32 quantizedSampleTick = 0;

		bool applyRootMotion = false;
		bool consumeRootMotionInPose = false;
		EZ::u32 rootMotionBoneIndex = 0;

		bool operator==(const SharedPaletteKey& rhs) const
		{
			return skeletonAsset == rhs.skeletonAsset
				&& skinAsset == rhs.skinAsset
				&& clipAsset == rhs.clipAsset
				&& applyScaleKeys == rhs.applyScaleKeys
				&& quantizedSampleTick == rhs.quantizedSampleTick
				&& applyRootMotion == rhs.applyRootMotion
				&& consumeRootMotionInPose == rhs.consumeRootMotionInPose
				&& rootMotionBoneIndex == rhs.rootMotionBoneIndex;

		}
	};

	struct SharedPaletteKeyHash
	{
		size_t operator()(const SharedPaletteKey& key) const noexcept
		{
			size_t h = 0;
			auto hashCombine = [&](size_t v)
				{
					h ^= v + 0x9e3779b9 + (h << 6) + (h >> 2);
				};

			hashCombine(std::hash<const void*>{}(key.skeletonAsset));
			hashCombine(std::hash<const void*>{}(key.skinAsset));
			hashCombine(std::hash<const void*>{}(key.clipAsset));
			hashCombine(std::hash<bool>{}(key.applyScaleKeys));
			hashCombine(std::hash<EZ::u32>{}(key.quantizedSampleTick));
			return h;
		}
	};

private:
	std::unordered_map<
		SharedPaletteKey,
		std::vector<DataProtocol::Mat4>,
		SharedPaletteKeyHash
	> m_SharedPaletteCache;
};

#endif