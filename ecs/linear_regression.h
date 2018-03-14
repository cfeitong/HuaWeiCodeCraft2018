#ifndef __LINEAR_REGRESSION_H__
#define __LINEAR_REGRESSION_H__

#include <vector>
using namespace std;

typedef pair<double, vector<double>> pdvd;

struct Sample {
    vector<int> X;
    int y;
};

class LinearRegression {
public:
    bool init(int n, vector<Sample> ts);
    pdvd loss(double reg);
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