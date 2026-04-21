#pragma once

class PIDController {
public:
    PIDController(float p, float i, float d, float maxInt = 100.0f, float maxOut = 100.f);

    float Update(float desired, float actual, float time);

private:
    float kp = 0.0f;
    float ki = 0.0f;
    float kd = 0.0f;

    float integralError = 0.0f;
    float previousError = 0.0f;

    float maxIntegral = 0.0f;
    float maxOutput = 0.0f;
};
