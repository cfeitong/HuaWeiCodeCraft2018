#include "predict.h"
#include "allocation.h"
#include "data_preprocess.h"
#include "linear_regression.h"
#include "evaluation.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <cstdio>
#include <string>
#include <cstring>

using namespace std;

string join(char **data, int count);


//你要完成的功能总入口
void predict_server(char *info[MAX_INFO_NUM], char *data[MAX_DATA_NUM],
                    int data_num, char *filename) {

    Info meta(info);
    int n = 5;
    const int DAYS_PER_BLOCK = meta.days;

    RecordSet records = RecordSet(parse_records(join(data, data_num)));
    SampleByFlavor samples = records.to_samples(n, 5);
    Allocator alloc(meta.cpu_lim, meta.mem_lim);
    for (const auto &flavor : meta.targets) {
        unique_ptr<LinearRegression> lr(new LinearRegression());
        auto &s = samples[flavor];
        lr->init(n, s);
        lr->train(200, 1e-3, 1e-1);
        auto pred = records.to_data(10, DAYS_PER_BLOCK, flavor);
        double ans = lr->predict(pred);
        ans *= meta.days / (1. * DAYS_PER_BLOCK);
        int dd = ceil(ans);
        for (int i = 0; i < dd; i++) {
            alloc.alloc(flavor);
        }
    }

    auto scores = evaluate(alloc, meta, "data/example/TestData_2015.2.20_2015.2.27.txt");
    int phy = alloc.count_phy();
    for (int i = 0; i < phy; i++) {
        printf("on phy machine %d, mem = %d, cpu = %d\n", i, meta.mem_lim, meta.cpu_lim);
        auto t = alloc.count(i);
        for (auto it : t) {
            printf("%s: %d\n", it.first.c_str(), it.second);
        }
        puts("-------------------------------------------------------");
    }
    printf("cpu score = %.2f\n", scores.first);
    printf("mem score = %.2f\n", scores.second);

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
