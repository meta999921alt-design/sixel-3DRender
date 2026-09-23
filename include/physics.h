#pragma once
#include <vector>
#include "vector3.h"
#include "color.h"

struct Ball {
    Vector3 pos, vel;
    double radius;
    Color color;
    double orbitRadius, orbitDir;

    Ball(Vector3 p, double r, Color c, double orbitR, double dir);
};

class World {
public:
    World();
    void update(double dt);
    const std::vector<Ball>& balls() const { return balls_; }

private:
    void spawn(double orbitR, double angle, double h, double r, Color c, double dir);
    void orbit(double dt);
    void collide();

    static void collidePair(Ball& a, Ball& b);
    static void collideStatic(Ball& b, const Vector3& center, double r);
    static void collidePlane(Ball& b, double planeY, double dir);

    static constexpr double kRestitution = 0.85;

    std::vector<Ball> balls_;
};
