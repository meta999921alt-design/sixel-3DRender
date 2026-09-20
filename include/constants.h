#pragma once
#include "vector3.h"

constexpr double kPi = 3.14159265358979;

// 물리/씬 양쪽에서 쓰는 아레나 기준점
inline const Vector3 kArenaCenter(0.0, -0.2, 3.0);
constexpr double kArenaRadius = 0.9;
constexpr double kFloorY = -1.4;
constexpr double kCeilingY = 4.0;
