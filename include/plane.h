#pragma once
#include <cmath>
#include "hittable.h"

class Plane : public Hittable {
public:
    Plane(double y, Color a, Color b, double refl, Vector3 n = { 0, 1, 0 })
        : y_(y), a_(a), b_(b), refl_(refl), n_(n) {}

    bool intersect(const Ray& ray, double& t, Vector3& hitPoint, Vector3& normal) const override {
        if (std::fabs(ray.direction.y) < 1e-6) return false;
        double tt = (y_ - ray.origin.y) / ray.direction.y;
        if (tt <= 0.001) return false;
        t = tt;
        hitPoint = ray.origin + ray.direction * tt;
        normal = n_;
        return true;
    }

    Color colorAt(const Vector3& p) const override {
        int cx = (int)std::floor(p.x), cz = (int)std::floor(p.z);
        bool light = (((cx % 2) + 2) % 2) == (((cz % 2) + 2) % 2);
        return light ? a_ : b_;
    }

    double reflectivity() const override { return refl_; }

private:
    double y_;
    Color a_, b_;
    double refl_;
    Vector3 n_;
};
