//
// Created by 陈斐童 on 30/03/2018.
//

#ifndef ECS_ARIMA_H
#define ECS_ARIMA_H

#include <vector>
#include <cmath>
#include <memory>

using namespace std;

enum ModelType {
    MA,
    AR,
    ARMA,
};

vector<vector<double>> yw_solve(vector<double> garma);

vector<double> auto_cov(const vector<double> &data, int p);
vector<double> auto_corr(const vector<double> &data, int p);
vector<vector<double>> mutal_corr(const vector<vector<double>> &data);

double average(const vector<double> &data);
double variance(const vector<double> &data);
double standard_err(const vector<double> &data);
double pearson_corr(const vector<double> &data_fir, const vector<double> &data_sec);

double norm_dist(double mu, double sigma);
vector<double> diff(const vector<double> &data, int order);

class Model {
  public:
    virtual vector<vector<double>> fit(const vector<double> &data) = 0;
    virtual double compute_aic(const vector<double> &data) = 0;
    virtual double predict(const vector<double> &data) = 0;
};

class ARModel : public Model {
  private:
    int p;
    vector<vector<double>> coe;

  public:
    ARModel(int _p) : p(_p) {}
    vector<vector<double>> fit(const vector<double> &data) override;
    vector<vector<double>> compute_coe(const vector<double> &data);
    double compute_aic(const vector<double> &data) override;
    double predict(const vector<double> &data) override;
};

class MAModel : public Model {
  private:
    int q;
    vector<vector<double>> coe;

  public:
    MAModel(int _q) : q(_q) {}
    vector<vector<double>> compute_coe(const vector<double> &data);
    vector<vector<double>> fit(const vector<double> &data) override;
    double compute_aic(const vector<double> &data) override;
    double predict(const vector<double> &data) override;
};

class ARMAModel : public Model {
  private:
    int p, q;
    vector<vector<double>> coe; // first AR, second MA

  public:
    ARMAModel(int _p, int _q) : p(_p), q(_q) {}
    vector<vector<double>> compute_coe(const vector<double> &data);
    vector<vector<double>> fit(const vector<double> &data);
    double compute_aic(const vector<double> &data);
    double predict(const vector<double> &data);
};

class ARIMAModel : public Model {
  private:
    int d, p, q;
    unique_ptr<Model> model;

  public:
    ARIMAModel(int _d, int _p, int _q) : d(_d), p(_p), q(_q) {
        if (_p != 0 && _q == 0) this->model = unique_ptr<Model>(new ARModel(_p));
        else if (_p == 0 && _q != 0) this->model = unique_ptr<Model>(new MAModel(_q));
        else this->model = unique_ptr<Model>(new ARMAModel(_p, _q));
    }
    vector<vector<double>> fit(const vector<double> &data);
    double compute_aic(const vector<double> &data);
    void estimate_params(const vector<double> &data);
    double predict(const vector<double> &data);
};

#endif //ECS_ARIMA_H
