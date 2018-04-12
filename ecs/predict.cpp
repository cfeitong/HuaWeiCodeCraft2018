#include "predict.h"
#include "allocation.h"
#include "data_preprocess.h"
#include "linear_regression.h"
#include "evaluation.h"
#include "matrix.h"
#include "simplex.h"
#include "kalman.h"
#include "utils.h"

#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cmath>
#include <algorithm>

using namespace std;

string join(char **data, int count);

int get_flavor_id(string flavor);

bool distribute(vector<int> &flavor, int cpu_tol, int mem_tol, int N, vector<vector<int>> &ans);


//你要完成的功能总入口
void predict_server(char *info[MAX_INFO_NUM], char *data[MAX_DATA_NUM],
                    int data_num, char *filename) {

    Info meta(info);
    meta.block_count = 4;
    meta.k = 0.15;
    INFO = meta;

    RecordSet records = RecordSet(parse_records(join(data, data_num)));
    vector<double> all_data;
    map<string, vector<Sample>> samples = records.to_samples();
    for (const auto &flavor : meta.targets) {
        auto pred = records.to_data(flavor);
        all_data.insert(all_data.end(), pred.end() - meta.block_count, pred.end());
    }
    Allocator alloc(meta.cpu_lim, meta.mem_lim, meta.opt_type);
    for (const auto &flavor : meta.targets) {
//        unique_ptr<LinearRegression> lr(new LinearRegression());
//        lr->init(meta.targets.size() * meta.block_count, samples[flavor]);
//        double loss = lr->train(2000, 1e-3, 1e-3);
//        double ans = lr->predict(all_data);

        // use kalman filter to predict
        auto data = records.to_data(flavor);
        print_vector(data);
        double ans = KalmanPred(data);
        print_vector(data);
//        cout << flavor << " " << ans << endl;
        for (int i = 0; i < round(ans) + 7.1; i++) alloc.add_elem(flavor);
    }

    alloc.compute();
    alloc.postprocess();
    Outputor output(alloc, meta);


    // 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
    write_result(output.get_output(), filename);
}

string join(char **data, int count) {
    stringstream ss;
    for (int i = 0; i < count; i++) {
        if (data[i] == nullptr)
            break;
        ss << data[i];
    }
    string s = ss.str();
    return s;
}

int get_flavor_id(string flavor) {
    // e.g. flavor11 return 11
    if (flavor.size() > 7) {
        return (flavor[6] - '0') * 10 + flavor[7] - '0';
    } else return flavor[6] - '0';
}

int flavor_cpu[] = {1, 1, 1, 2, 2, 2, 4, 4, 4, 8, 8, 8, 16, 16, 16};
int flavor_mem[] = {1, 2, 4, 2, 4, 8, 4, 8, 16, 8, 16, 32, 16, 32, 64};

bool distribute(vector<int> &flavor, int cpu_tol, int mem_tol, int N, vector<vector<int>> &ans) {
    assert(N < 1000);
    int interval = N * 15 + 1;
    Mat mat(2 * N + 30, N * 15 + 1);
    Mat goal_mat(1, N * 15 + 1);
    for (int i = 0; i < N; i++) {
        int s = i * 15 + 1;
        int t = i * 15 + 15;
        mat.mat[i][0] = cpu_tol;
        mat.mat[i + N][0] = mem_tol;
        for (int j = s; j <= t; j++) {
            mat.mat[i][j] = flavor_cpu[j - s];
            mat.mat[i + N][j] = flavor_mem[j - s];
        }
    }
    int st = 2 * N;
    for (int i = 0; i < 15; i++) {
        mat.mat[i + st][0] = flavor[i];
        mat.mat[i + 15 + st][0] = -flavor[i];
        for (int j = 1; j < interval; j++) {
            if ((j - i - 1) % 15 == 0) {
                mat.mat[st + i][j] = 1;
                mat.mat[st + i + 15][j] = -1;
            }
        }
    }

    vector<int> solution;
    initSimplexModel(mat, goal_mat);
    bool isok = SimplexRun();
    if (!isok) {
        return false;
    }
    // int max = getMaxIntValue();
    getIntSolution(solution);
    ans.clear();
    for (int i = 0; i < N; i++) {
        vector<int> tmp;
        for (int j = 0; j < 15; j++) {
            tmp.push_back(solution[i * 15 + j]);
        }
        ans.push_back(tmp);
    }

    return true;
}

