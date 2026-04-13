#pragma once
#ifndef __D_P_SKELETONASSET_H__
#define __D_P_SKELETONASSET_H__

#include "core/Types.h"
#include <string>
#include <vector>

#include "MathTypes.h"

namespace DataProtocol
{
	struct BoneDesc
	{
		static constexpr const char* TypeName() noexcept { return "BoneDesc"; }
		static constexpr EZ::u32 Version() noexcept { return 2; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("name", name);
			v("parentIndex", parentIndex);

			// 绑定姿势（本地空间）
			v("localBindPosition", localBindPosition);
			v("localBindRotation", localBindRotation);
			v("localBindScale", localBindScale);
		}

		std::string name;
		EZ::i32 parentIndex = -1;

		// bind pose / rest pose，本地TRS
		Vec3 localBindPosition{ 0.0f, 0.0f, 0.0f };
		Quat localBindRotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		Vec3 localBindScale{ 1.0f, 1.0f, 1.0f };
	};

	struct SkeletonAsset
	{
		static constexpr const char* TypeName() noexcept { return "SkeletonAsset"; }
		static constexpr EZ::u32 Version() noexcept { return 2; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("sourcePath", sourcePath);
			v("cookedAssetPath", cookedAssetPath);
			v("isImported", isImported);

			v("rootBoneIndex", rootBoneIndex);
			v("bones", bones);
		}

		std::string sourcePath;
		std::string cookedAssetPath;
		bool isImported = false;

		EZ::i32 rootBoneIndex = -1;
		std::vector<BoneDesc> bones;
	};
}

#endif