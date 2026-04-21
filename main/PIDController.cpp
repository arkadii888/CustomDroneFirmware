#include "PIDContorller.h"

PIDController::PIDController(float p, float i, float d, float maxInt, float maxOut) : kp(p),
ki(i), kd(d), maxIntegral(maxInt), maxOutput(maxOut) {}

float PIDController::Update(float desired, float actual, float time) {
    float error = desired - actual;
    float pOut = kp * error;

    integralError += error * time;
    if(integralError > maxIntegral) {
        integralError = maxIntegral;
    }
    if(integralError < -maxIntegral) {
        integralError = -maxIntegral;
    }
    float iOut = ki * integralError;

    float derevative = (error - previousError) / time;
    float dOut = kd * derevative;
    previousError = error;

    float output = pOut + iOut + dOut;
    if(output > maxOutput) {
        output = maxOutput;
    }
    if(output < -maxOutput) {
        output = -maxOutput;
    }

    return output;
}
