#pragma once

#include <limits>
#include <cstdint>
#include <cstddef>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

using usize = size_t;
using isize = ptrdiff_t;

constexpr u8 U8_MAX = std::numeric_limits<u8>::max();
constexpr u8 U8_MIN = std::numeric_limits<u8>::min();
constexpr u16 U16_MAX = std::numeric_limits<u16>::max();
constexpr u16 U16_MIN = std::numeric_limits<u16>::min();
constexpr u32 U32_MAX = std::numeric_limits<u32>::max();
constexpr u32 U32_MIN = std::numeric_limits<u32>::min();
constexpr u64 U64_MAX = std::numeric_limits<u64>::max();
constexpr u64 U64_MIN = std::numeric_limits<u64>::min();

constexpr i8 I8_MAX = std::numeric_limits<i8>::max();
constexpr i8 I8_MIN = std::numeric_limits<i8>::min();
constexpr i16 I16_MAX = std::numeric_limits<i16>::max();
constexpr i16 I16_MIN = std::numeric_limits<i16>::min();
constexpr i32 I32_MAX = std::numeric_limits<i32>::max();
constexpr i32 I32_MIN = std::numeric_limits<i32>::min();
constexpr i64 I64_MAX = std::numeric_limits<i64>::max();
constexpr i64 I64_MIN = std::numeric_limits<i64>::min();

constexpr f32 F32_MAX = std::numeric_limits<f32>::max();
constexpr f32 F32_MIN = std::numeric_limits<f32>::min();
constexpr f32 F32_EPSILON = std::numeric_limits<f32>::epsilon();

constexpr f64 F64_MAX = std::numeric_limits<f64>::max();
constexpr f64 F64_MIN = std::numeric_limits<f64>::min();
constexpr f64 F64_EPSILON = std::numeric_limits<f64>::epsilon();

constexpr usize USIZE_MAX = std::numeric_limits<usize>::max();
constexpr usize USIZE_MIN = std::numeric_limits<usize>::min();
constexpr isize ISIZE_MAX = std::numeric_limits<isize>::max();
constexpr isize ISIZE_MIN = std::numeric_limits<isize>::min();

template<typename T> inline void unused(T &&) {}
#define UNUSED(x) unused(x)

using EntityID = u32;
static constexpr EntityID ENTITY_NULL = std::numeric_limits<EntityID>::max();

static constexpr f32 GRAVITY = -32.0f;