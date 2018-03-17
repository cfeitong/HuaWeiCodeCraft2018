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

char final_result[10000];
static char s[100];

//你要完成的功能总入口
void predict_server(char *info[MAX_INFO_NUM], char *data[MAX_DATA_NUM],
                    int data_num, char *filename) {

    int line = 0;
    int cpu, mem, disk;
    sscanf(info[line++], "%d%d%d", &cpu, &mem, &disk);
    int needed;
    line++;
    sscanf(info[line++], "%d", &needed);
    vector<string> targets;
    for (int i = 0; i < needed; i++) {
        sscanf(info[line++], "%s", s);
        targets.emplace_back(s);
    }
    line++;
    sscanf(info[line++], "%s", s); string type(s);
    line++;
    int sy, sm, sd; sscanf(info[line++], "%d-%d-%d", &sy, &sm, &sd);
    int ey, em, ed; sscanf(info[line++], "%d-%d-%d", &ey, &em, &ed);
    int len = (ey - sy) * 365 + (em - sm) * 28 + ed - sd;

    mem *= 1024;
    int n = 10;
    RecordSet records = RecordSet(parse(join(data, data_num)));
    SampleByFlavor samples = records.to_samples(n);
    Allocator alloc(cpu, mem);
    for (auto flavor : targets) {
        unique_ptr<LinearRegression> lr(new LinearRegression());
        auto &s = samples[flavor];
        lr->init(n, s);
        lr->train(1000, 1e-3, 1e-1);
        auto pred = records.to_data(10, flavor);
        vector<double> ans = lr->predict(pred, len);
        for (double d : ans) {
            int dd = ceil(d);
            for (int i = 0; i < dd; i++) {
                alloc.alloc(flavor);
            }
        }
    }

    stringstream ss;
    int total = 0;
    for (string flavor : targets) {
        int cnt = alloc.count_vir(flavor);
        total += cnt;
    }
    ss << total << "\n";
    for (string flavor : targets) {
        int cnt = alloc.count_vir(flavor);
        ss << flavor << " " << cnt << "\n";
    }
    ss << "\n";

    int phy_count = alloc.count_phy();
    ss << phy_count << "\n";
    for (int i = 1; i <= phy_count; i++) {
        ss << i;
        for (string flavor : targets) {
            int cnt = alloc.count(i, flavor);
            ss << " " << flavor << " " << cnt;
        }
        ss << "\n";
    }
    string result = ss.str();

    strcpy(final_result, result.c_str());

    // 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
    write_result(final_result, filename);
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
