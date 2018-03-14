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

bool LinearRegression::train(int num_times, double lr, double reg) {
    for (int t = 1; t <= num_times; t++) {
        pdvd p = this->loss(reg);
        cout << "loss: " << p.first << endl;
        for (int i = 0; i < n; i++) {
            this->w[i] -= lr * p.second[i];
        }
    }
    return true;
}

vector<double> LinearRegression::predict(vector<double> testset, int len) {
    vector<double> pred, test;
    for (int i = testset.size() - this->n; i <= testset.size() - 1; i++)
        test.push_back(testset[i]);
    for (int i = 0; i < len; i++) {
        double score = 0;
        for (int i = test.size() - this->n; i <= test.size() - 1; i++) {
            score += test[i] * this->w[i];
        }
        pred.push_back(score);
        test.push_back(score);
    }
    return pred;
}

void LinearRegression::show() {
    for (int i = 0; i < this->n; i++)
        cout << this->w[i] << endl;
}
