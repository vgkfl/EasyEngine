#pragma once
#ifndef __D_P_ANIMATION_CLIP_ASSET_H__
#define __D_P_ANIMATION_CLIP_ASSET_H__

#include "core/Types.h"
#include <string>
#include <vector>

#include "MathTypes.h"

namespace DataProtocol
{
	struct AnimationVecKey
	{
		static constexpr const char* TypeName() noexcept { return "AnimationVecKey"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("time", time);
			v("value", value);
		}

		EZ::f32 time = 0.0f;
		Vec3 value{};
	};

	struct AnimationQuatKey
	{
		static constexpr const char* TypeName() noexcept { return "AnimationQuatKey"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("time", time);
			v("value", value);
		}

		EZ::f32 time = 0.0f;
		Quat value{};
	};

	struct BoneAnimationTrack
	{
		static constexpr const char* TypeName() noexcept { return "BoneAnimationTrack"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("boneName", boneName);
			v("skeletonBoneIndex", skeletonBoneIndex);
			v("positionKeys", positionKeys);
			v("rotationKeys", rotationKeys);
			v("scaleKeys", scaleKeys);
		}

		std::string boneName;
		EZ::i32 skeletonBoneIndex = -1;

		std::vector<AnimationVecKey> positionKeys;
		std::vector<AnimationQuatKey> rotationKeys;
		std::vector<AnimationVecKey> scaleKeys;
	};

	struct AnimationClipAsset
	{
		static constexpr const char* TypeName() noexcept { return "AnimationClipAsset"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("sourcePath", sourcePath);
			v("cookedAssetPath", cookedAssetPath);
			v("isImported", isImported);

			v("clipName", clipName);
			v("duration", duration);
			v("ticksPerSecond", ticksPerSecond);

			v("tracks", tracks);
		}

		std::string sourcePath;
		std::string cookedAssetPath;
		bool isImported = false;

		std::string clipName;
		EZ::f32 duration = 0.0f;
		EZ::f32 ticksPerSecond = 30.0f;

		std::vector<BoneAnimationTrack> tracks;
	};
}

#endif