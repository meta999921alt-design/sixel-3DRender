#pragma once
#include "ray.h"
#include "color.h"

class Hittable {
public:
    virtual ~Hittable() = default;
    virtual bool intersect(const Ray& ray, double& t, Vector3& hitPoint, Vector3& normal) const = 0;
    virtual Color colorAt(const Vector3& point) const = 0;
    virtual double reflectivity() const = 0;
};
