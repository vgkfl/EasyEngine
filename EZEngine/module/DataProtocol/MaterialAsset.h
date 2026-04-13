#pragma once
#ifndef __D_P_MATERIALASSET_H__
#define __D_P_MATERIALASSET_H__

#include <string>
#include <vector>

#include "core/Types.h"
#include "DataProtocol/MathTypes.h"

namespace DataProtocol
{
	enum class MaterialShadingModel : EZ::u8
	{
		Unlit = 0,
		Lit,
		Character
	};

	enum class MaterialCullMode : EZ::u8
	{
		None = 0,
		Front,
		Back
	};

	enum class MaterialBlendMode : EZ::u8
	{
		Opaque = 0,
		AlphaBlend,
		Additive
	};

	struct MaterialTextureSlot
	{
		static constexpr const char* TypeName() noexcept { return "MaterialTextureSlot"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("slotName", slotName);
			v("imageAssetPath", imageAssetPath);
			v("enabled", enabled);
			v("srgb", srgb);
		}

		std::string slotName;
		std::string imageAssetPath;
		bool enabled = true;
		bool srgb = true;
	};

	struct MaterialScalarParam
	{
		static constexpr const char* TypeName() noexcept { return "MaterialScalarParam"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("name", name);
			v("value", value);
		}

		std::string name;
		EZ::f32 value = 0.0f;
	};

	struct MaterialVectorParam
	{
		static constexpr const char* TypeName() noexcept { return "MaterialVectorParam"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("name", name);
			v("value", value);
		}

		std::string name;
		Vec4 value{};
	};

	struct MaterialAsset
	{
		static constexpr const char* TypeName() noexcept { return "MaterialAsset"; }
		static constexpr EZ::u32 Version() noexcept { return 2; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("sourcePath", sourcePath);
			v("cookedAssetPath", cookedAssetPath);
			v("isImported", isImported);

			// 新增：默认shader路径
			v("shaderPath", shaderPath);

			v("shadingModel", shadingModel);
			v("cullMode", cullMode);
			v("blendMode", blendMode);

			v("depthTest", depthTest);
			v("depthWrite", depthWrite);
			v("doubleSided", doubleSided);

			v("baseColor", baseColor);
			v("textureSlots", textureSlots);
			v("scalarParams", scalarParams);
			v("vectorParams", vectorParams);
		}

		std::string sourcePath;
		std::string cookedAssetPath;
		bool isImported = false;

		// 例如:
		// Project/Test01/res/shaders/StaticLit.shader
		// Project/Test01/res/shaders/CharacterLit.shader
		std::string shaderPath;

		MaterialShadingModel shadingModel = MaterialShadingModel::Lit;
		MaterialCullMode cullMode = MaterialCullMode::Back;
		MaterialBlendMode blendMode = MaterialBlendMode::Opaque;

		bool depthTest = true;
		bool depthWrite = true;
		bool doubleSided = false;

		Vec4 baseColor{ 1.0f, 1.0f, 1.0f, 1.0f };

		std::vector<MaterialTextureSlot> textureSlots;
		std::vector<MaterialScalarParam> scalarParams;
		std::vector<MaterialVectorParam> vectorParams;
	};
}

#endif