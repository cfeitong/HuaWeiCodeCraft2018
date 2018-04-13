#include "predict.h"
#include "allocation.h"
#include "data_preprocess.h"
#include "linear_regression.h"
#include "evaluation.h"
#include "common.h"
#include "kalman.h"
#include "exponential_smoothing.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <cstdio>
#include <string>
#include <cstring>
#include <cassert>
#include <spline.h>

using namespace std;

string join(char **data, int count);

int get_flavor_id(string flavor);

bool distribute(vector<int> &flavor, int cpu_tol, int mem_tol, int N, vector<vector<int>> &ans);


//你要完成的功能总入口
void predict_server(char *info[MAX_INFO_NUM], char *data[MAX_DATA_NUM],
                    int data_num, char *filename) {

    Info meta(info);
    int n = 4;
    const int DAYS_PER_BLOCK = 7;

    RecordSet records = RecordSet(parse_records(join(data, data_num)));
    SampleByFlavor samples = records.to_samples(n, DAYS_PER_BLOCK);
    Allocator alloc(meta.cpu_lim, meta.mem_lim, meta.opt_type);
    for (const auto &flavor : meta.targets) {
        // linear regression
        unique_ptr<LinearRegression> lr(new LinearRegression());
        vector<double> blockdata = records.to_data(DAYS_PER_BLOCK, flavor);
        pred = vector<double>(blockdata.end() - n, blockdata.end());
        auto &s = samples[flavor];
        lr->init(n, s);
        lr->train(3000, 1e-4, 1e-3);
        double ans0 = lr->predict(pred);
        // kalman filter
        double ans1 = KalmanPred(pred);
        // use last data 
        double ans3 = pred[pred.size() - 1];
        // exponential_smoothing
        double_exponential_smoothing<double, 1> dbexpsmth;
        dbexpsmth.set_1st_smoothing_constant(0.8);
        dbexpsmth.set_2nd_smoothing_constant(0.5);
        //dbexpsmth.set_vacillation_tolerance(0.1);
        es_vec<double, 1> curr_query, smth_result;
        for (auto &i : blockdata) {
            curr_query[0] = i;
            smth_result = dbexpsmth.push_to_pop(curr_query);
        }
        double ans4 = smth_result[0];

        // get flavor id
//        int dd = int((ans0 + ans1 + ans3 * (meta.days * 1.0 / DAYS_PER_BLOCK)) / 3 + 0.5);
        int dd = int(ans4 * (meta.days * 1.0 / DAYS_PER_BLOCK) + 0.5);
        for (int i = 0; i < max(dd, 1); i++) {
            alloc.add_elem(flavor);
        }
    }
    alloc.compute();
    alloc.postprocess();
    Outputor output(alloc, meta);
    write_result(output.get_output(), filename);

//    // 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
//    write_result(output.get_another_output(ans, meta), filename);
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

