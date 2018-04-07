#include "linear_regression.h"
#include <iostream>
#include <random>
#include <vector>
using namespace std;

const double eps = 1e-6;

LinearRegression::LinearRegression(int n, const vector<Sample> &ts) {
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
}

pdvd LinearRegression::loss(double reg) {
    double l = 0;
    vector<double> grad;
    for (int i = 0; i < n; i++)
        grad.push_back(0);

    for (auto it : this->trainset) {
        double score = 0;
        for (int i = 0; i < this->n; i++) {
            score += it.X[i] * this->w[i] * this->i[i];
        }
        l += (score - it.y) * (score - it.y) / 2;
        for (int i = 0; i < n; i++) {
            grad[i] += (score - it.y) * it.X[i];
        }
    }
    l = l / this->trainset.size();
    for (int i = 0; i < n; i++)
        l += reg * this->w[i] * this->w[i] * this->i[i] * this->i[i];
    for (int i = 0; i < n; i++) {
        grad[i] /= this->trainset.size();
        grad[i] += 2 * reg * this->w[i] * this->i[i];
    }
    return pdvd(l, grad);
}

pdd LinearRegression::norm(Sample &sample) {
    double mn = 0;
    double var = 0;
    int N = sample.X.size();
    for (auto &i : sample.X) mn += i;
    mn = mn / N;
    for(auto &i : sample.X) {
        var += (i - mn) * (i - mn);
    }
    var = sqrt(var / N);
    if (abs(var) < eps) {
        for (auto &i : sample.X) i = i - mn;
        sample.y = sample.y - mn;
        return pdd(mn, var);
    }
    for(auto &i : sample.X) {
        i = (i - mn) / var;
    }
    sample.y = (sample.y - mn) / var;
    return pdd(mn, var);
}

double LinearRegression::train(int num_times, double lr, double reg) {
    double loss = 0;
    for (auto &it : this->trainset) this->norm(it);
    for (int t = 1; t <= num_times; t++) {
        pdvd p = this->loss(reg);
//        cout << "loss: " << p.first << endl;
        for (int i = 0; i < n; i++) {
            this->w[i] -= lr * p.second[i] * this->i[i];
        }
        loss = p.first;
    }
    return loss;
}

double LinearRegression::predict(vector<double> testset) {
    vector<double> test;
    for (size_t i = testset.size() - this->n; i <= testset.size() - 1; i++)
        test.push_back(testset[i]);
    Sample tmp;
    tmp.X = test; tmp.y = 0;
    pdd p = this->norm(tmp);
    test = tmp.X;
    double score = 0;
    for (size_t i = test.size() - this->n; i <= test.size() - 1; i++) {
        score += test[i] * this->w[i] * this->i[i];
    }
    if (abs(p.second) < eps) score = score + p.first;
    else score = score * p.second + p.first;
    return score;
}

void LinearRegression::show() {
    for (int i = 0; i < this->n; i++)
        cout << this->w[i] << endl;
}

void LinearRegression::importance_norm(int seq, int len) {
    this->i.clear();
    for (int i = 0; i < this->n; i++) {
        this->i.push_back(1);
    }
    for (int i = seq*len; i < seq*(len+1); i++) {
        this->i[i] *= 10;
    }
    double sum = accumulate(this->i.begin(), this->i.end(), 0.);
    for (auto &v : this->i) {
        v /= sum;
    }
}
