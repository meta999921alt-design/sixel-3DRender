#pragma once
#include <vector>
#include <memory>
#include "hittable.h"
#include "material.h"
#include "physics.h"

class Scene {
public:
    void add(std::unique_ptr<Hittable> obj);
    void addLight(Light l);
    void setTime(double t);
    Color trace(const Ray& ray, int depth) const;

private:
    Color sky(const Ray& ray) const;
    static double starHash(double x, double y, double z);
    Color shade(const Ray& ray, const Hittable& obj, const Vector3& p, const Vector3& n, int depth) const;
    Color illuminate(const Light& light, const Vector3& p, const Vector3& n, const Vector3& view, const Color& base) const;
    bool blocked(const Vector3& p, const Vector3& n, const Vector3& dir, double dist) const;
    static double fresnel(double base, double cosTheta);

    std::vector<std::unique_ptr<Hittable>> objects_;
    std::vector<Light> lights_;
    double time_ = 0;
};

Scene buildScene(double time, const World& world);
