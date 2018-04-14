#ifndef _KALMAN_H_
#define _KALMAN_H_

#include <stdio.h>
#include <vector>

using namespace std;
class Kalman {
public:
    Kalman(double _Q, double _R) : Q(_Q), R(_R) {
        A = 1;
        H = 1;
        P = 10;
        filter_value = 0;
    }

    double filter(double last) {
        double value = A * filter_value;
        P = A * A * P + Q;
        double pre_value = filter_value;
        kalman_gain = P * H / (P * H * H + R);
        filter_value = value + (last - value) * kalman_gain;
        P = (1 - kalman_gain * H) * P;
        return filter_value;
    }

    double filter(vector<double> &data) {
        double a = 0;
        for (auto &i : data) {
            a = i = filter(i);
        }
        return a;
    }

private:
    double filter_value;
    double kalman_gain;
    double A, H, Q, R, P;
};

#endif