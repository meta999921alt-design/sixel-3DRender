#pragma once
#include <cmath>

struct Vector3 {
    double x = 0, y = 0, z = 0;

    Vector3() = default;
    Vector3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    Vector3 operator+(const Vector3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    Vector3 operator-(const Vector3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    Vector3 operator*(double f) const { return { x * f, y * f, z * f }; }
    Vector3 operator/(double f) const { return { x / f, y / f, z / f }; }

    double dot(const Vector3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vector3 cross(const Vector3& o) const {
        return { y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x };
    }
    double length() const { return std::sqrt(dot(*this)); }
    Vector3 normalized() const {
        double len = length();
        if (len <= 0.0) return { 0, 0, 0 };
        return *this / len;
    }
};
