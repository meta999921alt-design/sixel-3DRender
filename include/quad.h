#pragma once
#include <cmath>
#include "hittable.h"

class Quad : public Hittable {
public:
    Quad(Vector3 center, Vector3 uDir, double uLen, Vector3 vDir, double vLen, Color color, double reflectivity)
        : center_(center), uDir_(uDir.normalized()), vDir_(vDir.normalized()),
          uLen_(uLen), vLen_(vLen), color_(color), reflectivity_(reflectivity) {
        normal_ = uDir_.cross(vDir_).normalized();
    }

    bool intersect(const Ray& ray, double& t, Vector3& hitPoint, Vector3& normal) const override {
        double denom = ray.direction.dot(normal_);
        if (std::fabs(denom) < 1e-6) return false;
        double tt = (center_ - ray.origin).dot(normal_) / denom;
        if (tt <= 0.001) return false;

        Vector3 p = ray.origin + ray.direction * tt;
        Vector3 local = p - center_;
        double lu = local.dot(uDir_);
        double lv = local.dot(vDir_);
        if (std::fabs(lu) > uLen_ || std::fabs(lv) > vLen_) return false;

        t = tt;
        hitPoint = p;
        normal = normal_;
        return true;
    }

    Color colorAt(const Vector3&) const override { return color_; }
    double reflectivity() const override { return reflectivity_; }

private:
    Vector3 center_, uDir_, vDir_, normal_;
    double uLen_, vLen_;
    Color color_;
    double reflectivity_;
};
