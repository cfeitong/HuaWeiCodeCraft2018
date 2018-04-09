#ifndef __LINEAR_REGRESSION_H__
#define __LINEAR_REGRESSION_H__

#include "data_preprocess.h"
#include <vector>

using namespace std;

typedef pair<double, vector<double>> pdvd;
typedef pair<double, double> pdd;
pdd norm(Sample &sample);

class LinearRegression {
public:
    bool init(int n, const vector<Sample> &ts);
    pdvd loss(double reg);
    double train(int num_times, double lr, double reg);
    double predict(const vector<double> &testset);
    void show();
private:
    int n; // dating back n times
    vector<double> w;  // weight, size: n
    double b; // bias
    vector<Sample> trainset; // idx for date, value for number
};

#endif