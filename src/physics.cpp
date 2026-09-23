#include "physics.h"
#include <algorithm>

RigidBody::RigidBody(Vector3 pos, double r, double m, Color c, double restitution_, double friction_)
    : position(pos), velocity(0, 0, 0), radius(r), mass(m), restitution(restitution_), friction(friction_), color(c) {}

void PhysicsEngine::addBody(RigidBody body) {
    bodies_.push_back(body);
    initialBodies_.push_back(body);
}

void PhysicsEngine::addPlane(const StaticPlane& plane) {
    planes_.push_back(plane);
}

void PhysicsEngine::reset() {
    bodies_ = initialBodies_;
}

void PhysicsEngine::step(double dt) {
    applyGravity(dt);
    integrate(dt);
    resolveCollisions(dt);
}

void PhysicsEngine::applyGravity(double dt) {
    for (auto& b : bodies_) {
        if (b.invMass() <= 0.0) continue;
        b.velocity = b.velocity + gravity * dt;
    }
}

void PhysicsEngine::integrate(double dt) {
    for (auto& b : bodies_) b.position = b.position + b.velocity * dt;
}

void PhysicsEngine::resolveCollisions(double dt) {
    for (size_t i = 0; i < bodies_.size(); i++)
        for (size_t j = i + 1; j < bodies_.size(); j++)
            resolveBodyPair(bodies_[i], bodies_[j]);

    for (auto& b : bodies_)
        for (auto& p : planes_)
            resolvePlaneContact(b, p, dt);
}

void PhysicsEngine::resolvePlaneContact(RigidBody& body, const StaticPlane& plane, double dt) {
    double dist = (body.position - plane.point).dot(plane.normal);
    double penetration = body.radius - dist;
    if (penetration <= 0.0) return;

    body.position = body.position + plane.normal * penetration;

    double vn = body.velocity.dot(plane.normal);
    Vector3 normalVel = plane.normal * vn;
    Vector3 tangentVel = body.velocity - normalVel;

    if (vn < 0.0) {
        double restitution = std::min(body.restitution, plane.restitution);
        normalVel = normalVel * -restitution;
    }

    // 접촉 중인 동안 접선 속도를 감쇠 (단순화된 운동마찰 모델)
    double frictionCoeff = std::max(body.friction, plane.friction);
    double frictionFactor = std::clamp(1.0 - frictionCoeff * dt * 12.0, 0.0, 1.0);
    tangentVel = tangentVel * frictionFactor;

    body.velocity = normalVel + tangentVel;
}

void PhysicsEngine::resolveBodyPair(RigidBody& a, RigidBody& b) {
    Vector3 delta = b.position - a.position;
    double dist = delta.length();
    double minDist = a.radius + b.radius;
    if (dist >= minDist || dist < 1e-6) return;

    double invA = a.invMass(), invB = b.invMass();
    double totalInv = invA + invB;
    if (totalInv <= 0.0) return;

    Vector3 n = delta / dist;
    double overlap = minDist - dist;
    a.position = a.position - n * (overlap * invA / totalInv);
    b.position = b.position + n * (overlap * invB / totalInv);

    double vn = (b.velocity - a.velocity).dot(n);
    if (vn > 0.0) return;

    double restitution = std::min(a.restitution, b.restitution);
    double impulseMag = -(1.0 + restitution) * vn / totalInv;
    Vector3 impulse = n * impulseMag;

    a.velocity = a.velocity - impulse * invA;
    b.velocity = b.velocity + impulse * invB;
}
