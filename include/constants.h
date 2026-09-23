#pragma once
#include <cmath>
#include "vector3.h"

constexpr double kPi = 3.14159265358979;
constexpr double kFloorY = -1.4;

// 경사로(램프) 기하 — 물리 충돌과 렌더링 양쪽에서 공유
constexpr double kRampAngle = 0.6109; // 35도
constexpr double kRampLength = 5.0;
constexpr double kRampWidth = 3.0;
inline const Vector3 kRampBase(0.0, kFloorY, 4.0); // 카메라(원점, +Z 방향을 봄) 정면에 위치
inline const Vector3 kRampSlopeDir(-std::cos(kRampAngle), std::sin(kRampAngle), 0.0);
inline const Vector3 kRampNormal(std::sin(kRampAngle), std::cos(kRampAngle), 0.0);
