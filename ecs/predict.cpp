#include "predict.h"
#include "allocation.h"
#include "data_preprocess.h"
#include "linear_regression.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdio.h>
#include <string>
#include <cstring>

using namespace std;

string join(char **data, int count);


//你要完成的功能总入口
void predict_server(char *info[MAX_INFO_NUM], char *data[MAX_DATA_NUM],
                    int data_num, char *filename) {

    int n = 10;
    Info meta(info);

    RecordSet records = RecordSet(parse(join(data, data_num)));
    SampleByFlavor samples = records.to_samples(n);
    Allocator alloc(meta.cpu_lim, meta.mem_lim);
    for (auto flavor : meta.targets) {
        unique_ptr<LinearRegression> lr(new LinearRegression());
        auto &s = samples[flavor];
        lr->init(n, s);
        lr->train(1000, 1e-3, 1e-1);
        auto pred = records.to_data(10, flavor);
        vector<double> ans = lr->predict(pred, meta.days);
        for (double d : ans) {
            int dd = ceil(d);
            for (int i = 0; i < dd; i++) {
                alloc.alloc(flavor);
            }
        }
    }

    Outputor output(alloc, meta);

    // 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
    write_result(output.get_output(), filename);
}

string join(char **data, int count) {
    stringstream ss;
    for (int i = 0; i < count; i++) {
        if (data[i] == NULL)
            break;
        ss << data[i];
    }
    string s = ss.str();
    return s;
}
