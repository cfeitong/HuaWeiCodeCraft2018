#ifndef __LINEAR_REGRESSION_H__
#define __LINEAR_REGRESSION_H__

#include "data_preprocess.h"
#include <vector>

using namespace std;

typedef pair<double, vector<double>> pdvd;
typedef pair<double, double> pdd;

class LinearRegression {
public:
    bool init(int n, const vector<Sample>& ts, const vector<double>& initwb);
    pdvd loss(double reg, int sgd=-1);
    pdd norm(Sample &sample);
    bool train(int num_times, double lr, double reg, int sgd=-1);
    double predict(vector<double> testset);
    vector<double> get_w();
    double get_b();
    void show();
private:
    int n; // dating back n times
    vector<double> w;  // weight, size: n
    double b; // bias
    vector<Sample> trainset; // idx for date, value for number
    vector<Sample> sampletrainset;
};

#endif