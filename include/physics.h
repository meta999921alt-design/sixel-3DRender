#pragma once
#include <vector>
#include "vector3.h"
#include "color.h"

struct RigidBody {
    Vector3 position;
    Vector3 velocity;
    double radius;
    double mass;
    double restitution;
    double friction;
    Color color;

    RigidBody(Vector3 pos, double r, double m, Color c, double restitution_, double friction_);

    // mass <= 0 이면 무한질량(고정체)으로 취급 (지금은 안 쓰지만 확장 대비)
    double invMass() const { return mass > 0.0 ? 1.0 / mass : 0.0; }
};

struct StaticPlane {
    Vector3 point;
    Vector3 normal; // 정규화되어 있어야 함
    double restitution;
    double friction;
};

class PhysicsEngine {
public:
    void addBody(RigidBody body);
    void addPlane(const StaticPlane& plane);

    void step(double dt);
    void reset(); // 처음 addBody했을 때의 위치/속도로 되돌림

    const std::vector<RigidBody>& bodies() const { return bodies_; }

    Vector3 gravity{ 0.0, -9.8, 0.0 };

private:
    void applyGravity(double dt);
    void integrate(double dt);
    void resolveCollisions(double dt);
    void resolvePlaneContact(RigidBody& body, const StaticPlane& plane, double dt);
    void resolveBodyPair(RigidBody& a, RigidBody& b);

    std::vector<RigidBody> bodies_;
    std::vector<RigidBody> initialBodies_;
    std::vector<StaticPlane> planes_;
};
