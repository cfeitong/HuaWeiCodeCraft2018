//
// Created by 陈斐童 on 30/03/2018.
//

#include "analys.h"

#include <cassert>
#include <cmath>
#include <random>

using namespace std;

vector<vector<double>> yw_solve(vector<double> garma) {
    assert(garma.size() > 1);
    size_t order = garma.size();
    vector<vector<double>> result(order, vector<double>(order));
    vector<double> sigma_sq(order);
    order--;

    sigma_sq[0] = garma[0];
    result[1][1] = garma[1] / sigma_sq[0];
    sigma_sq[1] = sigma_sq[0] * (1 - result[1][1] * result[1][1]);
    for (int k = 1; k < order; k++) {
        double sum_top = 0, sum_sub = 0;
        for (int j = 1; j <= k; j++) {
            sum_top += garma[k + 1 - j] * result[k][j];
            sum_sub += garma[j] * result[k][j];
        }
        result[k + 1][k + 1] = (garma[k + 1] - sum_top) / (garma[0] - sum_sub);
        for (int j = 1; j <= k; j++) {
            result[k + 1][j] = result[k][j] - result[k + 1][k + 1] * result[k][k - j + 1];
        }
        sigma_sq[k + 1] = sigma_sq[k] * (1 - result[k + 1][k + 1] * result[k + 1][k + 1]);
    }
    result[0] = sigma_sq;
    return result;
}

vector<vector<double>> ARModel::compute_coe(const vector<double> &data) {
    vector<double> garma = auto_cov(data, p);

    vector<vector<double>> result = yw_solve(garma);
    vector<double> ar_coe((size_t) p + 1);
    for (int i = 0; i < p; i++) {
        ar_coe[i] = result[p][i + 1];
    }
    ar_coe[p] = result[0][p];

    return {ar_coe};
}

vector<vector<double>> ARModel::fit(const vector<double> &data) {
    coe = compute_coe(data);
    return {coe};
}

double ARModel::compute_aic(const vector<double> &data) {
    const vector<double> coe = this->coe[0];
    size_t p = coe.size();
    size_t n = data.size();
    double sum_err = 0;

    for (size_t i = p - 1; i < n; i++) {
        double tmp_ar = 0;
        for (int j = 0; j < p - 1; j++) {
            tmp_ar += coe[j] * data[i - j - 1];
        }
        sum_err += (data[i] - tmp_ar) * (data[i] - tmp_ar);
    }
    return (n - (p - 1)) * log(sum_err / (n - (p - 1))) + (p + 1) * 2;
}

double ARModel::predict(const vector<double> &data) {
    double result = 0;
    const vector<double> coe = this->coe[0];

    for (int k = p; k < data.size(); k++) {
        result = 0;
        for (int i = 0; i < p; i++) {
            result += coe[i] * data[k - i - 1];
        }
    }
    return result;
}

vector<vector<double>> MAModel::compute_coe(const vector<double> &data) {
    int p = 0;
    double min_aic = 1e200;
    int len = (int) data.size();
    len = min(len, 10);

    for (int i = 1; i < len; i++) {
        vector<double> garma = auto_cov(data, i);
        vector<vector<double>> result = yw_solve(garma);

        vector<double> ar_coe((size_t) i + 1);
        for (int k = 0; k < i; k++) {
            ar_coe[k] = result[i][k + 1];
        }
        ar_coe[i] = result[0][i];
        if (this->coe.empty()) coe.push_back(ar_coe);
        else coe[0] = ar_coe;
        double aic = compute_aic(data);
        if (aic < min_aic) {
            min_aic = aic;
            p = i;
        }
    }

    vector<double> best_garma = auto_cov(data, p);
    vector<vector<double>> best_result = yw_solve(best_garma);

    vector<double> alpha((size_t) p + 1);
    alpha[0] = -1;
    for (int i = 1; i <= p; i++) {
        alpha[i] = best_result[p][i];
    }

    vector<double> para_garma((size_t) q + 1);
    for (int k = 0; k <= q; k++) {
        double sum = 0;
        for (int j = 0; j <= p - k; j++) {
            sum += alpha[j] * alpha[k + j];
        }
        para_garma[k] = sum / best_result[0][p];
    }

    auto tmp = yw_solve(para_garma);
    vector<double> ma_coe((size_t) q + 1);
    for (int i = 1; i < ma_coe.size(); i++) {
        ma_coe[i] = tmp[q][i];
    }
    ma_coe[0] = tmp[0][q];

    return {ma_coe};
}

vector<vector<double>> MAModel::fit(const vector<double> &data) {
    coe = compute_coe(data);
    return coe;
}

double MAModel::compute_aic(const vector<double> &data) {
    const vector<double> coe = this->coe[0];
    size_t q = coe.size(), n = data.size();
    double tmp_ma = 0, sum_err = 0;
    vector<double> err(q);

    for (size_t i = q - 1; i < n; i++) {
        tmp_ma = 0;
        for (size_t j = 1; j < q; j++) {
            tmp_ma += coe[j] * err[j];
        }

        for (size_t j = q - 1; j > 0; j--) {
            err[j] = err[j - 1];
        }

        err[0] = norm_dist(0, 1) * sqrt(coe[0]);
        sum_err += (data[i] - tmp_ma) * (data[i] - tmp_ma);
    }
    return (n - (q - 1)) * log(sum_err / (n - (q - 1))) + (q + 1) * 2;
}

double MAModel::predict(const vector<double> &data) {
    double result = 0;
    vector<double> err((size_t) q + 1);
    const vector<double> coe = this->coe[0];
    for (int k = q; k < data.size(); k++) {
        result = 0;
        for (int i = 1; i <= q; i++) {
            result += coe[i] * err[i];
        }
        //产生各个时刻的噪声
        for (int j = q; j > 0; j--) {
            err[j] = err[j - 1];
        }
        err[0] = norm_dist(0, 1) * sqrt(coe[0]);
    }
    return result;
}

vector<vector<double>> ARMAModel::compute_coe(const vector<double> &data) {
    vector<double> all_garma = auto_cov(data, p + q);
    vector<double> garma((size_t) p + 1);
    for (int i = 0; i < garma.size(); i++) {
        garma[i] = all_garma[q + i];
    }
    vector<vector<double>> ar_result = yw_solve(garma);

    // AR
    vector<double> ar_coe((size_t) p + 1);
    for (int i = 0; i < p; i++) {
        ar_coe[i] = ar_result[p][i + 1];
    }
    ar_coe[p] = ar_result[0][p];

    // MA
    vector<double> alpha((size_t) p + 1);
    alpha[0] = -1;
    for (int i = 1; i <= p; i++) {
        alpha[i] = ar_coe[i - 1];
    }

    vector<double> para_garma((size_t) q + 1);
    for (int k = 0; k <= q; k++) {
        double sum = 0;
        for (int i = 0; i <= p; i++) {
            for (int j = 0; j <= p; j++) {
                sum += alpha[i] * alpha[j] * all_garma[abs(k + i - j)];
            }
        }
        para_garma[k] = sum;
    }
    vector<vector<double>> ma_result = yw_solve(para_garma);
    vector<double> ma_coe((size_t) q + 1);
    for (int i = 1; i <= q; i++) {
        ma_coe[i] = ma_result[q][i];
    }
    ma_coe[0] = ma_result[0][q];


    vector<double> arma_coe((size_t) p + q + 2);
    for (int i = 0; i < arma_coe.size(); i++) {
        if (i < ar_coe.size()) {
            arma_coe[i] = ar_coe[i];
        } else {
            arma_coe[i] = ma_coe[i - ar_coe.size()];
        }
    }
    return {arma_coe};
}

vector<vector<double>> ARMAModel::fit(const vector<double> &data) {
    auto arma_coe = compute_coe(data)[0];
    auto base = arma_coe.begin();
    vector<double> ar_coe(base, base + p + 1);
    vector<double> ma_coe(base + p + 1, base + p + q + 2);

    coe.push_back(ar_coe);
    coe.push_back(ma_coe);

    return coe;
}

double ARMAModel::compute_aic(const vector<double> &data) {
    size_t n = data.size();
    const vector<double> &ar_coe = coe[0];
    const vector<double> &ma_coe = coe[1];
    vector<double> err((size_t) q);
    size_t p = ar_coe.size();
    size_t q = ma_coe.size();
    double tmp_ar = 0, tmp_ma = 0, sum_err = 0;

    for (size_t i = p - 1; i < n; i++) {
        tmp_ar = 0.0;
        for (int j = 0; j < p - 1; j++) {
            tmp_ar += ar_coe[j] * data[i - j - 1];
        }
        tmp_ma = 0.0;
        for (int j = 1; j < q; j++) {
            tmp_ma += ma_coe[j] * err[j];
        }

        for (size_t j = q - 1; j > 0; j--) {
            err[j] = err[j - 1];
        }
        err[0] = norm_dist(0, 1) * sqrt(ma_coe[0]);

        sum_err += (data[i] - tmp_ar - tmp_ma) * (data[i] - tmp_ar - tmp_ma);
    }
    return (n - (q + p - 1)) * log(sum_err / (n - (q + p - 1))) + (p + q) * 2;

}

double ARMAModel::predict(const vector<double> &data) {
    double ar_result = 0, ma_result = 0;
    vector<double> err((size_t) q + 1);
    const vector<double> &ar_coe = coe[0];
    const vector<double> &ma_coe = coe[1];
    for (int k = p; k < data.size(); k++) {
        ar_result = 0;
        ma_result = 0;
        for (int i = 0; i < p; i++) {
            ar_result += ar_coe[i] * data[k - i - 1];
        }
        for (int i = 1; i <= q; i++) {
            ma_result += ma_coe[i] * err[i];
        }

        //产生各个时刻的噪声
        for (int j = q; j > 0; j--) {
            err[j] = err[j - 1];
        }

        err[0] = norm_dist(0, 1) * sqrt(ma_coe[0]);
    }
    return ar_result + ma_result;
}

vector<vector<double>> ARIMAModel::fit(const vector<double> &data) {
    vector<double> diff_data = diff(data, this->d);
    return this->model->fit(diff_data);
}

double ARIMAModel::compute_aic(const vector<double> &data) {
    vector<double> diff_data = diff(data, this->d);
    return this->model->compute_aic(diff_data);
}

void ARIMAModel::estimate_params(const vector<double> &data) {
    double min_aic = 1e200;
    for (int p = 0; p < 5; p++) {
        for (int q = 0; q < 5; q++) {
            if (p == 0 && q == 0) continue;
            for (int d = 0; d < 5; d++) {
                ARIMAModel model(d, p, q);
                model.fit(data);
                double aic = model.compute_aic(data);
                if (aic < min_aic) {
                    min_aic = aic;
                    *this = move(model);
                }
            }
        }
    }
}

double ARIMAModel::predict(const vector<double> &data) {
    vector<double> diff_data = diff(data, this->d);
    double result = this->model->predict(diff_data);
    result += data[data.size() - d];
    return result;
}

vector<double> auto_cov(const vector<double> &data, int p) {
    double mean = average(data);
    vector<double> cov((size_t) p + 1);

    for (int i = 0; i <= p; i++) {
        cov[i] = 0;
        for (int j = 0; j < data.size() - i; j++) {
            cov[i] += (data[i + j] - mean) * (data[j] - mean);
        }
        cov[i] /= (data.size() - 1);
    }

    return cov;
}

vector<double> auto_corr(const vector<double> &data, int p) {
    vector<double> cov = auto_cov(data, p);
    vector<double> corr((size_t) p + 1);
    double var = variance(data);

    if (var != 0) {
        for (int i = 0; i < corr.size(); i++) {
            corr[i] = cov[i] / var;
        }
    }

    return corr;
}

vector<vector<double>> mutal_corr(const vector<vector<double>> &data) {
    vector<vector<double>> ret(data.size(), vector<double>(data.size()));
    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data.size(); j++) {
            ret[i][j] = pearson_corr(data[i], data[j]);
        }
    }
    return ret;
}

double average(const vector<double> &data) {
    double ret = 0;
    for (const auto &v : data) {
        ret += v;
    }
    return ret / data.size();
}

double variance(const vector<double> &data) {
    assert(data.size() > 1);
    double var = 0;
    double mean = average(data);

    for (int i = 0; i < data.size(); i++) {
        var += (data[i] - mean) * (data[i] - mean);
    }
    var /= data.size() - 1;
    return var;
}

double standard_err(const vector<double> &data) {
    return sqrt(variance(data));
}

double pearson_corr(const vector<double> &data_fir, const vector<double> &data_sec) {
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x_sq = 0, sum_y_sq = 0;
    size_t len = min(data_fir.size(), data_sec.size());
    for (int i = 0; i < len; i++) {
        sum_x += data_fir[i];
        sum_y += data_sec[i];
        sum_xy += data_fir[i] * data_sec[i];
        sum_x_sq += data_fir[i] * data_fir[i];
        sum_y_sq += data_sec[i] * data_sec[i];
    }

    double numerator = sum_xy - sum_x * sum_y / len;
    double denominator = sqrt((sum_x_sq - sum_x * sum_x / len) * (sum_y_sq - sum_y * sum_y / len));

    if (abs(denominator) < 1e-6) {
        return 0;
    }

    return numerator / denominator;
}

double norm_dist(double mu, double sigma) {
    random_device rd;
    mt19937 gen(rd());
    normal_distribution<double> dist(mu, sigma);
    return dist(gen);
}

vector<double> diff(const vector<double> &data, int order) {
    assert(data.size() > order);
    vector<double> ret;
    for (int i = 0; i < data.size() - order; i++) {
        ret.push_back(data[i + order] - data[i]);
    }
    return ret;
}

