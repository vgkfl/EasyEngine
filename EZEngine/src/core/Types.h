#pragma once

#ifndef __TYPES_H__
#define __TYPES_H__

#include <cstdint>
#include <cstddef>

namespace EZ
{
	using u8 = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;

	using i8 = int8_t;
	using i16 = int16_t;
	using i32 = int32_t;
	using i64 = int64_t;

	using usize = size_t;    // 无符号尺寸（数组、容器大小）
	using isize = ptrdiff_t; // 有符号尺寸（偏移、差值）

	using f32 = float;
	using f64 = double;

	using c8 = char;       // 普通字符
	using s8 = int8_t;     // 有符号字符

	using byte = std::byte; //代表一字节内存

	using ptr = uintptr_t;   // 能存下任何指针的无符号整数
	using iptr = intptr_t;    // 有符号版本
}

#endif