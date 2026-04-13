#pragma once

#ifndef __D_P_MESHASSET_H__
#define __D_P_MESHASSET_H__

#include "core/Types.h"
#include <vector>
#include <string>

#include "MathTypes.h"

namespace DataProtocol
{
	enum class VertexSemantic : EZ::u8
	{
		Position,
		Normal,
		Tangent,
		Bitangent,
		UV,
		Color,
		BoneIndices,
		BoneWeights
	};

	inline const char* ToString(VertexSemantic value)
	{
		switch (value)
		{
		case VertexSemantic::Position:    return "Position";
		case VertexSemantic::Normal:      return "Normal";
		case VertexSemantic::Tangent:     return "Tangent";
		case VertexSemantic::Bitangent:   return "Bitangent";
		case VertexSemantic::UV:          return "UV";
		case VertexSemantic::Color:       return "Color";
		case VertexSemantic::BoneIndices: return "BoneIndices";
		case VertexSemantic::BoneWeights: return "BoneWeights";
		default:                          return "Unknown";
		}
	}

	enum class VertexScalarType : EZ::u8
	{
		Float32,
		Int32,
		UInt32,
		UInt16,
		UInt8
	};

	inline const char* ToString(VertexScalarType value)
	{
		switch (value)
		{
		case VertexScalarType::Float32: return "Float32";
		case VertexScalarType::Int32:   return "Int32";
		case VertexScalarType::UInt32:  return "UInt32";
		case VertexScalarType::UInt16:  return "UInt16";
		case VertexScalarType::UInt8:   return "UInt8";
		default:                        return "Unknown";
		}
	}

	enum class IndexScalarType : EZ::u8
	{
		UInt16,
		UInt32
	};

	inline const char* ToString(IndexScalarType value)
	{
		switch (value)
		{
		case IndexScalarType::UInt16: return "UInt16";
		case IndexScalarType::UInt32: return "UInt32";
		default:                      return "Unknown";
		}
	}

	struct VertexAttributeDesc
	{
		static constexpr const char* TypeName() noexcept { return "VertexAttributeDesc"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("semantic", semantic);
			v("semanticIndex", semanticIndex);
			v("componentCount", componentCount);
			v("scalarType", scalarType);
			v("normalized", normalized);
			v("offset", offset);
		}

		VertexSemantic semantic = VertexSemantic::Position;
		EZ::u8 semanticIndex = 0;
		EZ::u8 componentCount = 0;
		VertexScalarType scalarType = VertexScalarType::Float32;
		bool normalized = false;
		EZ::u32 offset = 0;
	};

	struct VertexBufferLayoutDesc
	{
		static constexpr const char* TypeName() noexcept { return "VertexBufferLayoutDesc"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("attributes", attributes);
			v("stride", stride);
		}

		std::vector<VertexAttributeDesc> attributes;
		EZ::u32 stride = 0;
	};

	struct SubMeshDesc
	{
		static constexpr const char* TypeName() noexcept { return "SubMeshDesc"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("indexOffset", indexOffset);
			v("indexCount", indexCount);
			v("materialSlot", materialSlot);
		}

		EZ::u32 indexOffset = 0;
		EZ::u32 indexCount = 0;
		EZ::u32 materialSlot = 0;
	};

	struct AABBDesc
	{
		static constexpr const char* TypeName() noexcept { return "AABBDesc"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("min", min);
			v("max", max);
		}

		Vec3 min{ 0.0f, 0.0f, 0.0f };
		Vec3 max{ 0.0f, 0.0f, 0.0f };
	};

	struct MeshAsset
	{
		static constexpr const char* TypeName() noexcept { return "MeshAsset"; }
		static constexpr EZ::u32 Version() noexcept { return 2; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("sourcePath", sourcePath);
			v("cookedAssetPath", cookedAssetPath);
			v("isImported", isImported);

			v("vertexCount", vertexCount);
			v("vertexLayout", vertexLayout);
			v("vertexRawData", vertexRawData);

			v("indexCount", indexCount);
			v("indexType", indexType);
			v("indexRawData", indexRawData);

			v("subMeshes", subMeshes);
			v("bounds", bounds);

			v("hasSkinning", hasSkinning);
		}

		std::string sourcePath;
		std::string cookedAssetPath;
		bool isImported = false;

		EZ::u32 vertexCount = 0;
		VertexBufferLayoutDesc vertexLayout;
		std::vector<EZ::u8> vertexRawData;

		EZ::u32 indexCount = 0;
		IndexScalarType indexType = IndexScalarType::UInt32;
		std::vector<EZ::u8> indexRawData;

		std::vector<SubMeshDesc> subMeshes;
		AABBDesc bounds;

		bool hasSkinning = false;
	};
}

#endif