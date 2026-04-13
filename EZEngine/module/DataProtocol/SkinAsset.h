#pragma once
#ifndef __D_P_SKINASSET_H__
#define __D_P_SKINASSET_H__

#include "core/Types.h"
#include <string>
#include <vector>

#include "MathTypes.h"

namespace DataProtocol
{
	struct SkinBoneBindingDesc
	{
		static constexpr const char* TypeName() noexcept { return "SkinBoneBindingDesc"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("boneName", boneName);
			v("skeletonBoneIndex", skeletonBoneIndex);
			v("inverseBindMatrix", inverseBindMatrix);
		}

		std::string boneName;
		EZ::i32 skeletonBoneIndex = -1;
		Mat4 inverseBindMatrix{};
	};

	struct SkinAsset
	{
		static constexpr const char* TypeName() noexcept { return "SkinAsset"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("sourcePath", sourcePath);
			v("cookedAssetPath", cookedAssetPath);
			v("isImported", isImported);
			v("meshAssetPath", meshAssetPath);
			v("skeletonAssetPath", skeletonAssetPath);
			v("maxBonesPerVertex", maxBonesPerVertex);
			v("boneBindings", boneBindings);
		}

		std::string sourcePath;
		std::string cookedAssetPath;
		bool isImported = false;

		std::string meshAssetPath;
		std::string skeletonAssetPath;
		EZ::u32 maxBonesPerVertex = 4;
		std::vector<SkinBoneBindingDesc> boneBindings;
	};
}

#endif