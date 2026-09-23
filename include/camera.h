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
    static Vector3 pitchIt(Vector3 v, double p);
    static Vector3 yawIt(Vector3 v, double yaw);

    int w_, h_;
    Vector3 pos_{ 0, 0, 0 };
    double yaw_ = 0, pitch_ = 0;
};
