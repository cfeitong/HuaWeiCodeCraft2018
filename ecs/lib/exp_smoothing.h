//
// Created by 陈斐童 on 14/04/2018.
//

#ifndef ECS_EXP_SMOOdoubleHING_H
#define ECS_EXP_SMOOdoubleHING_H

#include <cassert>

class ExpSmoothing {
public:
    /** Default constructor */
    ExpSmoothing(double _alpha, double _beta) :
            cur_smoothed(0),
            last_smoothed(0),
            cur_corr(0),
            alpha(_alpha),
            beta(_beta),
            cnt(0) {}


public:
    void set_alpha(double val) {
        assert(val > 0 && val < 1);
        alpha = val;
    }

    void set_beta(double val) {
        assert(val > 0 && val < 1);
        beta = val;
    }

    double smooth(double raw) {
        if (0 == cnt) {
            cur_smoothed = raw;
        } else if (1 == cnt) {
            cur_corr = raw - cur_smoothed;
            last_smoothed = cur_smoothed;
            cur_smoothed = alpha * raw + (1 - alpha) * (cur_smoothed + cur_corr);
        } else {
            cur_corr = beta * (cur_smoothed - last_smoothed) + (1 - beta) * cur_corr;
            last_smoothed = cur_smoothed;
            cur_smoothed = alpha * raw + (1 - alpha) * (cur_smoothed + cur_corr);
        }
        ++cnt;
        return cur_smoothed;
    }

private:
    double cur_smoothed;
    double last_smoothed;
    double cur_corr;
    double alpha;
    double beta;
    int cnt;
};

#endif //ECS_EXP_SMOOdoubleHING_H
