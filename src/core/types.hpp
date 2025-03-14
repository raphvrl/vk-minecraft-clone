#pragma once

#include <cstdint>
#include <limits>

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

#define UNUSED(x) (void)(x)

using EntityID = u32;
static constexpr EntityID ENTITY_NULL = std::numeric_limits<EntityID>::max();