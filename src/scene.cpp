#include "scene.h"
#include "constants.h"
#include "sphere.h"
#include "plane.h"
#include <cmath>
#include <algorithm>
#include <limits>

void Scene::add(std::unique_ptr<Hittable> obj) { objects_.push_back(std::move(obj)); }
void Scene::addLight(Light l) { lights_.push_back(l); }
void Scene::setTime(double t) { time_ = t; }

Color Scene::trace(const Ray& ray, int depth) const {
    if (depth > 2) return sky(ray);

    double closestT = std::numeric_limits<double>::max();
    const Hittable* hit = nullptr;
    Vector3 hitPoint, hitNormal;

    for (auto& obj : objects_) {
        double t;
        Vector3 p, n;
        if (obj->intersect(ray, t, p, n) && t < closestT) {
            closestT = t;
            hit = obj.get();
            hitPoint = p;
            hitNormal = n;
        }
    }

    if (!hit) return sky(ray);
    return shade(ray, *hit, hitPoint, hitNormal, depth);
}

Color Scene::sky(const Ray& ray) const {
    double factor = 0.5 * (ray.direction.y + 1.0);
    Color result(8 * (1 - factor) + 15 * factor, 12 * (1 - factor) + 25 * factor, 20 * (1 - factor) + 45 * factor);

    double gx = std::floor(ray.direction.x * 400.0);
    double gy = std::floor(ray.direction.y * 400.0);
    double gz = std::floor(ray.direction.z * 400.0);

    if (starHash(gx, gy, gz) > 0.9975) {
        double b = starHash(gx + 1, gy + 3, gz + 7);
        double twinkle = 0.5 + 0.5 * std::sin(time_ * 6.0 + b * 30.0);
        double add = b * twinkle * 255.0;
        result.r = std::min(255.0, result.r + add);
        result.g = std::min(255.0, result.g + add);
        result.b = std::min(255.0, result.b + add * 0.9);
    }
    return result;
}

double Scene::starHash(double x, double y, double z) {
    double h = std::sin(x * 12.9898 + y * 78.233 + z * 37.719) * 43758.5453;
    return h - std::floor(h);
}

Color Scene::shade(const Ray& ray, const Hittable& obj, const Vector3& p, const Vector3& n, int depth) const {
    Color base = obj.colorAt(p);
    double refl = obj.reflectivity();

    Vector3 view = (ray.origin - p).normalized();
    double cosView = std::max(0.0, n.dot(view));
    double effRefl = refl > 0.0 ? fresnel(refl, cosView) : 0.0;

    Color result = base * 0.15;
    for (auto& light : lights_) result = result + illuminate(light, p, n, view, base);

    if (effRefl > 0.0) {
        Vector3 dir = (ray.direction - n * (2.0 * ray.direction.dot(n))).normalized();
        Color reflected = trace(Ray(p + n * 0.001, dir), depth + 1);
        result = result * (1.0 - effRefl) + reflected * effRefl;
    }
    return result.clamped();
}

Color Scene::illuminate(const Light& light, const Vector3& p, const Vector3& n, const Vector3& view, const Color& base) const {
    Vector3 toLight = light.position - p;
    double dist = toLight.length();
    Vector3 dir = toLight / dist;

    if (blocked(p, n, dir, dist)) return { 0, 0, 0 };

    double diff = std::max(0.0, n.dot(dir));
    Vector3 refl = (n * (2.0 * n.dot(dir)) - dir).normalized();
    double spec = std::pow(std::max(0.0, view.dot(refl)), 16.0) * 0.6;

    return {
        (base.r * diff * 0.8 + 255.0 * spec) * light.intensity * light.tint.x,
        (base.g * diff * 0.8 + 255.0 * spec) * light.intensity * light.tint.y,
        (base.b * diff * 0.8 + 255.0 * spec) * light.intensity * light.tint.z
    };
}

bool Scene::blocked(const Vector3& p, const Vector3& n, const Vector3& dir, double dist) const {
    Ray r(p + n * 0.001, dir);
    double t;
    Vector3 pp, nn;
    for (auto& obj : objects_)
        if (obj->intersect(r, t, pp, nn) && t < dist) return true;
    return false;
}

double Scene::fresnel(double base, double cosTheta) {
    double f = std::pow(std::clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
    return base + (1.0 - base) * f;
}

Scene buildScene(double time, const World& world) {
    Scene scene;

    scene.add(std::make_unique<Sphere>(kArenaCenter, kArenaRadius, Material{ { 235, 235, 240 }, 0.55 }));
    scene.add(std::make_unique<Plane>(kFloorY, Color(205, 205, 210), Color(25, 25, 32), 0.25, Vector3(0, 1, 0)));
    scene.add(std::make_unique<Plane>(kCeilingY, Color(50, 50, 62), Color(30, 30, 38), 0.1, Vector3(0, -1, 0)));

    for (auto& ball : world.balls())
        scene.add(std::make_unique<Sphere>(ball.pos, ball.radius, Material{ ball.color, 0.15 }));

    scene.addLight({ { 3, 5, -4 }, { 1.0, 0.95, 0.85 }, 1.0 });
    scene.addLight({ { -4, 2.5, -1.5 }, { 0.4, 0.55, 1.0 }, 0.55 });

    scene.setTime(time);
    return scene;
}
