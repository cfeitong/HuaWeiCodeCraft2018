#ifndef __LINEAR_REGRESSION_H__
#define __LINEAR_REGRESSION_H__

#include "data_preprocess.h"
#include <vector>

using namespace std;

typedef pair<double, vector<double>> pdvd;
typedef pair<double, double> pdd;

class LinearRegression {
public:
    bool init(int n, vector<Sample> ts);
    pdvd loss(double reg);
    pdd norm(Sample &sample);
    bool train(int num_times, double lr, double reg);
    vector<double> predict(vector<double> testset, int len);
    void show();
private:
    int n; // dating back n times
    vector<double> w;  // weight, size: n
    double b; // bias
    vector<Sample> trainset; // idx for date, value for number
};

#endif