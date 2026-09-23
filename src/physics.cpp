#include "physics.h"
#include "constants.h"
#include <cmath>
#include <algorithm>

Ball::Ball(Vector3 p, double r, Color c, double orbitR, double dir)
    : pos(p), vel(0, 0, 0), radius(r), color(c), orbitRadius(orbitR), orbitDir(dir) {}

World::World() {
    spawn(2.2, 0.0, 0.4, 0.35, { 255, 60, 60 }, 1);
    spawn(2.4, 1.3, 0.1, 0.3, { 40, 180, 255 }, -1);
    spawn(2.8, 2.6, -0.3, 0.3, { 60, 220, 90 }, 1);
    spawn(2.0, 3.9, 0.2, 0.25, { 200, 80, 255 }, -1);
    spawn(3.0, 5.2, -0.15, 0.28, { 255, 200, 50 }, 1);
}

void World::update(double dt) {
    orbit(dt);
    for (auto& b : balls_) b.pos = b.pos + b.vel * dt;
    collide();
}

void World::spawn(double orbitR, double angle, double h, double r, Color c, double dir) {
    Vector3 p = kArenaCenter + Vector3(orbitR * std::cos(angle), h, orbitR * std::sin(angle));
    balls_.emplace_back(p, r, c, orbitR, dir);
}

void World::orbit(double dt) {
    static const Vector3 up(0, 1, 0);
    for (auto& b : balls_) {
        Vector3 toCenter = kArenaCenter - b.pos;
        double dist = toCenter.length();
        if (dist < 1e-4) continue;

        Vector3 radial = toCenter / dist;
        double err = dist - b.orbitRadius;
        // 반경으로 되돌리는 스프링 힘 + 계속 돌게 만드는 접선 힘
        Vector3 spring = radial * (4.0 * err);
        Vector3 tangent = up.cross(radial).normalized() * b.orbitDir * 2.5;

        b.vel = b.vel + (spring + tangent) * dt;
        b.vel = b.vel * std::max(0.0, 1.0 - 0.3 * dt);
    }
}

void World::collide() {
    for (size_t i = 0; i < balls_.size(); i++)
        for (size_t j = i + 1; j < balls_.size(); j++)
            collidePair(balls_[i], balls_[j]);

    for (auto& b : balls_) {
        collideStatic(b, kArenaCenter, kArenaRadius);
        collidePlane(b, kFloorY, 1.0);
        collidePlane(b, kCeilingY, -1.0);
    }
}

void World::collidePair(Ball& a, Ball& b) {
    Vector3 delta = b.pos - a.pos;
    double dist = delta.length();
    double minDist = a.radius + b.radius;
    if (dist >= minDist || dist < 1e-6) return;

    Vector3 n = delta / dist;
    double overlap = minDist - dist;
    a.pos = a.pos - n * (overlap * 0.5);
    b.pos = b.pos + n * (overlap * 0.5);

    double vn = (b.vel - a.vel).dot(n);
    if (vn > 0) return;

    Vector3 impulse = n * (-(1.0 + kRestitution) * vn / 2.0);
    a.vel = a.vel - impulse;
    b.vel = b.vel + impulse;
}

void World::collideStatic(Ball& b, const Vector3& center, double r) {
    Vector3 delta = b.pos - center;
    double dist = delta.length();
    double minDist = b.radius + r;
    if (dist >= minDist || dist < 1e-6) return;

    Vector3 n = delta / dist;
    b.pos = b.pos + n * (minDist - dist);

    double vn = b.vel.dot(n);
    if (vn < 0) b.vel = b.vel - n * (vn * (1.0 + kRestitution));
}

void World::collidePlane(Ball& b, double planeY, double dir) {
    double d = dir * (b.pos.y - planeY);
    double pen = b.radius - d;
    if (pen <= 0) return;

    b.pos.y += dir * pen;
    double vn = dir * b.vel.y;
    if (vn < 0) b.vel.y -= dir * vn * (1.0 + kRestitution);
}
