#pragma once
#ifndef __D_P_MATHTYPES_H__
#define __D_P_MATHTYPES_H__

#include "core/Types.h"
#include <array>

namespace DataProtocol
{
	// =========================================================
	// 基础向量类型（float）
	// =========================================================

	struct Vec2
	{
		static constexpr const char* TypeName() noexcept { return "Vec2"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("x", x);
			v("y", y);
		}

		EZ::f32 x = 0.0f;
		EZ::f32 y = 0.0f;
	};

	struct Vec3
	{
		static constexpr const char* TypeName() noexcept { return "Vec3"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("x", x);
			v("y", y);
			v("z", z);
		}

		EZ::f32 x = 0.0f;
		EZ::f32 y = 0.0f;
		EZ::f32 z = 0.0f;
	};

	struct Vec4
	{
		static constexpr const char* TypeName() noexcept { return "Vec4"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("x", x);
			v("y", y);
			v("z", z);
			v("w", w);
		}

		EZ::f32 x = 0.0f;
		EZ::f32 y = 0.0f;
		EZ::f32 z = 0.0f;
		EZ::f32 w = 0.0f;
	};

	// =========================================================
	// 基础向量类型（int）
	// =========================================================

	struct IVec2
	{
		static constexpr const char* TypeName() noexcept { return "IVec2"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("x", x);
			v("y", y);
		}

		EZ::i32 x = 0;
		EZ::i32 y = 0;
	};

	struct IVec3
	{
		static constexpr const char* TypeName() noexcept { return "IVec3"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("x", x);
			v("y", y);
			v("z", z);
		}

		EZ::i32 x = 0;
		EZ::i32 y = 0;
		EZ::i32 z = 0;
	};

	struct IVec4
	{
		static constexpr const char* TypeName() noexcept { return "IVec4"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("x", x);
			v("y", y);
			v("z", z);
			v("w", w);
		}

		EZ::i32 x = 0;
		EZ::i32 y = 0;
		EZ::i32 z = 0;
		EZ::i32 w = 0;
	};

	// =========================================================
	// 基础向量类型（uint）
	// =========================================================

	struct UVec2
	{
		static constexpr const char* TypeName() noexcept { return "UVec2"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("x", x);
			v("y", y);
		}

		EZ::u32 x = 0;
		EZ::u32 y = 0;
	};

	struct UVec3
	{
		static constexpr const char* TypeName() noexcept { return "UVec3"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("x", x);
			v("y", y);
			v("z", z);
		}

		EZ::u32 x = 0;
		EZ::u32 y = 0;
		EZ::u32 z = 0;
	};

	struct UVec4
	{
		static constexpr const char* TypeName() noexcept { return "UVec4"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("x", x);
			v("y", y);
			v("z", z);
			v("w", w);
		}

		EZ::u32 x = 0;
		EZ::u32 y = 0;
		EZ::u32 z = 0;
		EZ::u32 w = 0;
	};

	// =========================================================
	// 四元数
	// 与 glm::quat 一致，默认单位四元数 (0,0,0,1)
	// =========================================================

	struct Quat
	{
		static constexpr const char* TypeName() noexcept { return "Quat"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("x", x);
			v("y", y);
			v("z", z);
			v("w", w);
		}

		EZ::f32 x = 0.0f;
		EZ::f32 y = 0.0f;
		EZ::f32 z = 0.0f;
		EZ::f32 w = 1.0f;
	};

	// =========================================================
	// 矩阵
	// 采用列主序（column-major），和 glm 默认习惯一致
	// m[0] ~ m[3]   : 第0列
	// m[4] ~ m[7]   : 第1列
	// m[8] ~ m[11]  : 第2列
	// m[12] ~ m[15] : 第3列
	// =========================================================

	struct Mat3
	{
		static constexpr const char* TypeName() noexcept { return "Mat3"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("m", m);
		}

		std::array<EZ::f32, 9> m
		{
			1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f
		};
	};

	struct Mat4
	{
		static constexpr const char* TypeName() noexcept { return "Mat4"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("m", m);
		}

		std::array<EZ::f32, 16> m
		{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	};

	// =========================================================
	// 引擎里很常用的组合类型
	// =========================================================

	struct Transform
	{
		static constexpr const char* TypeName() noexcept { return "Transform"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("position", position);
			v("rotation", rotation);
			v("scale", scale);
		}

		Vec3 position{};
		Quat rotation{};
		Vec3 scale{ 1.0f, 1.0f, 1.0f };
	};

	struct AABB
	{
		static constexpr const char* TypeName() noexcept { return "AABB"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("min", min);
			v("max", max);
		}

		Vec3 min{};
		Vec3 max{};
	};

	struct Sphere
	{
		static constexpr const char* TypeName() noexcept { return "Sphere"; }
		static constexpr EZ::u32 Version() noexcept { return 1; }

		template<typename Visitor>
		void Reflect(Visitor&& v)
		{
			v("center", center);
			v("radius", radius);
		}

		Vec3 center{};
		EZ::f32 radius = 0.0f;
	};
}

#endif