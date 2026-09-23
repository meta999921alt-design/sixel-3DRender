#include "camera.h"
#include <cmath>
#include <algorithm>

Camera::Camera(int w, int h) : w_(w), h_(h) {}

void Camera::rotate(double dyaw, double dpitch) {
    yaw_ += dyaw;
    pitch_ = std::clamp(pitch_ + dpitch, -1.5, 1.5);
}

void Camera::move(double fwd, double right) {
    pos_ = pos_ + forward() * fwd + strafe() * right;
}

Vector3 Camera::forward() const { return { std::sin(yaw_), 0, std::cos(yaw_) }; }
Vector3 Camera::strafe() const { return { std::cos(yaw_), 0, -std::sin(yaw_) }; }

Ray Camera::rayForPixel(int x, int y) const {
    double sx = ((2.0 * (x + 0.5)) / w_ - 1.0) * 1.1;
    double sy = ((2.0 * (y + 0.5)) / h_ - 1.0) * 0.6;
    Vector3 dir = yawIt(pitchIt({ sx, sy, 1.0 }, pitch_), yaw_);
    return { pos_, dir.normalized() };
}

Vector3 Camera::pitchIt(Vector3 v, double p) {
    double c = std::cos(p), s = std::sin(p);
    return { v.x, v.y * c + v.z * s, -v.y * s + v.z * c };
}

Vector3 Camera::yawIt(Vector3 v, double yaw) {
    double c = std::cos(yaw), s = std::sin(yaw);
    return { v.x * c + v.z * s, v.y, -v.x * s + v.z * c };
}
