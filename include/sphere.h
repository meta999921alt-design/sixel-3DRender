#pragma once
#include <cmath>
#include "hittable.h"
#include "material.h"

class Sphere : public Hittable {
public:
    Sphere(Vector3 center, double radius, Material mat) : center_(center), radius_(radius), mat_(mat) {}

    bool intersect(const Ray& ray, double& t, Vector3& hitPoint, Vector3& normal) const override {
        Vector3 oc = ray.origin - center_;
        double a = ray.direction.dot(ray.direction);
        double b = 2.0 * oc.dot(ray.direction);
        double c = oc.dot(oc) - radius_ * radius_;
        double disc = b * b - 4 * a * c;
        if (disc < 0) return false;

        double sq = std::sqrt(disc);
        double t0 = (-b - sq) / (2 * a);
        double t1 = (-b + sq) / (2 * a);
        double chosen = (t0 > 0.001) ? t0 : ((t1 > 0.001) ? t1 : -1.0);
        if (chosen < 0) return false;

        t = chosen;
        hitPoint = ray.origin + ray.direction * t;
        normal = (hitPoint - center_).normalized();
        return true;
    }

    Color colorAt(const Vector3&) const override { return mat_.color; }
    double reflectivity() const override { return mat_.reflectivity; }

private:
    Vector3 center_;
    double radius_;
    Material mat_;
};
