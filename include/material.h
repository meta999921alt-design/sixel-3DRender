#pragma once
#include "vector3.h"
#include "color.h"

struct Material {
    Color color;
    double reflectivity;
};

struct Light {
    Vector3 position, tint;
    double intensity;
};
