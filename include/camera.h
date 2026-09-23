#pragma once
#include "ray.h"

class Camera {
public:
    Camera(int w, int h);

    void rotate(double dyaw, double dpitch);
    void move(double fwd, double right);

    Vector3 forward() const;
    Vector3 strafe() const;

    Ray rayForPixel(int x, int y) const;

private:
    void updateTrig();

    int w_, h_;
    Vector3 pos_{ 0, 0, 0 };
    double yaw_ = 0, pitch_ = 0;
    double cosYaw_ = 1, sinYaw_ = 0, cosPitch_ = 1, sinPitch_ = 0;
};
