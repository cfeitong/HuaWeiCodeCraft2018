#include "linear_regression.h"
#include <iostream>
#include <random>
#include <vector>
using namespace std;

bool LinearRegression::init(int n, vector<Sample> ts) {
    this->n = n;
    this->trainset = ts;
    random_device rd;
    mt19937 generator(rd());
    uniform_real_distribution<double> distribution(0, 0.1);
    for (int i = 0; i < n; i++) {
        double w = distribution(generator);
        this->w.push_back(w);
    }
    this->b = distribution(generator);
    return true;
}

pdvd LinearRegression::loss(double reg) {
    double l = 0;
    vector<double> grad;
    for (int i = 0; i < n; i++)
        grad.push_back(0);

    for (auto it : this->trainset) {
        double score = 0;
        for (int i = 0; i < this->n; i++) {
            score += it.X[i] * this->w[i];
        }
        l += (score - it.y) * (score - it.y) / 2;
        for (int i = 0; i < n; i++) {
            grad[i] += (score - it.y) * it.X[i];
        }
    }
    l = l / this->trainset.size();
    for (int i = 0; i < n; i++)
        l += reg * this->w[i] * this->w[i];
    for (int i = 0; i < n; i++) {
        grad[i] /= this->trainset.size();
        grad[i] += 2 * reg * this->w[i];
    }
    return pdvd(l, grad);
}

pdd LinearRegression::norm(Sample &sample) {
    double mn = 0;
    double var = 1e-6;
    int N = sample.X.size();
    for (auto &i : sample.X) mn += i;
    mn = mn / N;
    for(auto &i : sample.X) {
        var += (i - mn) * (i - mn);
    }
    var = sqrt(var / N);
    for(auto &i : sample.X) {
        i = (i - mn) / var;
    }
    sample.y = (sample.y - mn) / var;
    return pdd(mn, var);
}

bool LinearRegression::train(int num_times, double lr, double reg) {
    for (auto &it : this->trainset) this->norm(it);
    for (int t = 1; t <= num_times; t++) {
        pdvd p = this->loss(reg);
        cout << "loss: " << p.first << endl;
        for (int i = 0; i < n; i++) {
            this->w[i] -= lr * p.second[i];
        }
    }
    return true;
}

double LinearRegression::predict(vector<double> testset) {
    vector<double> test;
    for (int i = testset.size() - this->n; i <= testset.size() - 1; i++)
        test.push_back(testset[i]);
    Sample tmp;
    tmp.X = test; tmp.y = 0;
    pdd p = this->norm(tmp);
    test = tmp.X;
    double score = 0;
    for (int i = test.size() - this->n; i <= test.size() - 1; i++) {
        score += test[i] * this->w[i];
    }
    score = score * p.second + p.first;
    return score;
}

void LinearRegression::show() {
    for (int i = 0; i < this->n; i++)
        cout << this->w[i] << endl;
}