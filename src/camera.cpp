#include "camera.h"
#include <cmath>
#include <algorithm>

Camera::Camera(int w, int h) : w_(w), h_(h) {
    updateTrig();
}

void Camera::rotate(double dyaw, double dpitch) {
    yaw_ += dyaw;
    pitch_ = std::clamp(pitch_ + dpitch, -1.5, 1.5);
    updateTrig();
}

void Camera::updateTrig() {
    cosYaw_ = std::cos(yaw_);
    sinYaw_ = std::sin(yaw_);
    cosPitch_ = std::cos(pitch_);
    sinPitch_ = std::sin(pitch_);
}

void Camera::move(double fwd, double right) {
    pos_ = pos_ + forward() * fwd + strafe() * right;
}

Vector3 Camera::forward() const { return { sinYaw_, 0, cosYaw_ }; }
Vector3 Camera::strafe() const { return { cosYaw_, 0, -sinYaw_ }; }

Ray Camera::rayForPixel(int x, int y) const {
    double sx = ((2.0 * (x + 0.5)) / w_ - 1.0) * 1.1;
    double sy = -((2.0 * (y + 0.5)) / h_ - 1.0) * 0.6; // 화면 위쪽(y=0)이 world +Y(위)를 가리키도록 부호 반전

    // 프레임당 한 번만 계산된 캐시값을 그대로 씀 (픽셀마다 sin/cos 재계산 X)
    double py = sy * cosPitch_ + 1.0 * sinPitch_;
    double pz = -sy * sinPitch_ + 1.0 * cosPitch_;

    double dx = sx * cosYaw_ + pz * sinYaw_;
    double dz = -sx * sinYaw_ + pz * cosYaw_;

    return { pos_, Vector3(dx, py, dz).normalized() };
}
