#pragma once
#ifndef __D_P_IMAGEASSET_H__
#define __D_P_IMAGEASSET_H__

#include <string>

#include "core/Types.h"

namespace DataProtocol
{
	enum class ImageDimension : EZ::u8
	{
		Texture2D = 0,
		CubeMap
	};

	enum class ImageFormat : EZ::u8
	{
		Unknown = 0,
		R8,
		RG8,
		RGB8,
		RGBA8,
		RGBA8_SRGB,

		R16F,
		RG16F,
		RGBA16F
	};

	struct ImageAsset
	{
		static constexpr const char* TypeName() noexcept { return "ImageAsset"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("sourcePath", sourcePath);
			v("cookedAssetPath", cookedAssetPath);
			v("isImported", isImported);

			v("dimension", dimension);
			v("format", format);

			v("width", width);
			v("height", height);
			v("channels", channels);
			v("mipLevels", mipLevels);

			v("srgb", srgb);
			v("hasAlpha", hasAlpha);
		}

		std::string sourcePath;
		std::string cookedAssetPath;
		bool isImported = false;

		ImageDimension dimension = ImageDimension::Texture2D;
		ImageFormat format = ImageFormat::Unknown;

		EZ::u32 width = 0;
		EZ::u32 height = 0;
		EZ::u32 channels = 0;
		EZ::u32 mipLevels = 1;

		bool srgb = true;
		bool hasAlpha = false;
	};
}

#endif