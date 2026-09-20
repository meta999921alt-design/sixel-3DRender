#pragma once
#include "vector3.h"

struct Ray {
    Vector3 origin, direction;
    Ray() = default;
    Ray(Vector3 o, Vector3 d) : origin(o), direction(d) {}
};
