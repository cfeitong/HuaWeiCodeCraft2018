#include "predict.h"
#include "allocation.h"
#include "data_preprocess.h"
#include "linear_regression.h"
#include "evaluation.h"
#include "common.h"
#include "matrix.h"
#include "simplex.h"
#include "ARIMAModel.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <cstdio>
#include <string>
#include <cstring>
#include <cassert>

using namespace std;

string join(char **data, int count);

double predict(const vector<double> &dataArray);

int get_flavor_id(string flavor);

bool distribute(vector<int> &flavor, int cpu_tol, int mem_tol, int N, vector<vector<int>> &ans);


//你要完成的功能总入口
void predict_server(char *info[MAX_INFO_NUM], char *data[MAX_DATA_NUM],
                    int data_num, char *filename) {

    Info meta(info);
    int n = 3;
    const int DAYS_PER_BLOCK = meta.days;

    RecordSet records = RecordSet(parse_records(join(data, data_num)));
    SampleByFlavor samples = records.to_samples(n, 5);
    Allocator alloc(meta.cpu_lim, meta.mem_lim, meta.opt_type);
    vector<int> flavornum(15, 0);
    for (const auto &flavor : meta.targets) {
        auto pred = records.to_data(10, DAYS_PER_BLOCK, flavor);
        double ans = predict(pred);
        printf("predict ans = %.2f\n", ans);
        ans *= meta.days / (1. * DAYS_PER_BLOCK);
        flavornum[get_flavor_id(flavor) - 1] = (int) ans;
    }
    vector<vector<int>> ans;
    int l = 0, r = 200, N = -1;
    // use binary search to get answer
    while (l <= r) {
        int mid = (l + r) / 2;
        bool isok = distribute(flavornum, meta.cpu_lim, meta.mem_lim, mid, ans);
        //cout << "yes" << endl;
        if (isok) {
            r = mid - 1;
            N = mid;
        } else l = mid + 1;
    }
    Outputor output(alloc, meta);


    // 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
    write_result(output.get_another_output(ans, meta), filename);
}

double predict(const vector<double> &dataArray) {
    ARIMAModel *arima = new ARIMAModel(dataArray);
    int period = 1;
    int modelCnt = 5;
    int cnt = 0;
    std::vector<std::vector<int>> list;
    std::vector<int> tmpPredict(modelCnt);

    for (int k = 0; k < modelCnt; ++k) { //控制通过多少组参数进行计算最终的结果
        std::vector<int> bestModel = arima->getARIMAModel(period, list, k != 0);
        //std::cout<<bestModel.size()<<std::endl;

        if (bestModel.empty()) {
            tmpPredict[k] = (int) dataArray[dataArray.size() - period];
            cnt++;
            break;
        } else {
            //std::cout<<bestModel[0]<<bestModel[1]<<std::endl;
            int predictDiff = arima->predictValue(bestModel[0], bestModel[1], period);
            //std::cout<<"fuck"<<std::endl;
            tmpPredict[k] = arima->aftDeal(predictDiff, period);
            cnt++;
        }
        list.push_back(bestModel);
    }

    double sumPredict = 0.0;
    for (int k = 0; k < cnt; ++k) {
        sumPredict += ((double) tmpPredict[k]) / (double) cnt;
    }
    return sumPredict;
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
    MATRIX mat;
    MATRIX goal_mat;
    STRATEGY strategy = ILP;

    // int flavor[] = {1, 3, 2, 5, 6, 0, 8, 3, 2, 0, 11, 3, 7, 0, 0, 0};
    //int cpu_tol = 56;
    //int mem_tol = 128;
    //const int N = 6;
    assert(N < 1000);
    float *val = (float *) malloc((15001 * 2030) * sizeof(float));
    //float val[(10*15 + 1) * (2*10 + 30)];
    fill(val, val+(15001 * 2030), 0);
    int interval = N * 15 + 1;
    for (int i = 0; i < N; i++) {
        int s = i * 15 + 1;
        int t = i * 15 + 15;
        val[i * interval] = cpu_tol;
        val[(i + N) * interval] = mem_tol;
        for (int j = s; j <= t; j++) {
            val[i * interval + j] = flavor_cpu[j - s];
            val[(i + N) * interval + j] = flavor_mem[j - s];
        }
    }
    int st = interval * 2 * N;
    for (int i = 0; i < 15; i++) {
        val[st + i * interval] = flavor[i];
        val[st + (i + 15) * interval] = -flavor[i];
        for (int j = 1; j < interval; j++) {
            if ((j - i - 1) % 15 == 0) {
                val[st + i * interval + j] = 1;
                val[st + (i + 15) * interval + j] = -1;
            }
        }
    }

    //float goal[ * 15 + 1];
    float *goal = (float *) malloc((15001) * sizeof(float));
    fill(goal, goal+15001, 0);
    int max;
    int *solution = (int *) malloc((1500) * sizeof(int));
    //int solution[6 * 15];

    create_matrix(&mat, 2 * N + 30, interval);
    set_matrix(&mat, val);


    create_matrix(&goal_mat, 1, interval);
    set_matrix(&goal_mat, goal);

    initSimplexModel(mat, goal_mat, NULL);
    //printf("yes\n");
    u8 isok = SimplexRun(strategy);
    if (isok == 0) {
        printf("no solution!\n");
        return false;
    }
    max = getMaxIntValue();
    getIntSolution(solution);
    deleteSimplexModel();

    ans.clear();
    for (int i = 0; i < N; i++) {
        vector<int> tmp;
        for (int j = 0; j < 15; j++) {
            tmp.push_back(solution[i * 15 + j]);
            // printf("%d ", solution[i * 15 + j]);
        }
        // printf("\n");
        ans.push_back(tmp);
    }

    delete_matrix(&mat);
    delete_matrix(&goal_mat);

    return true;
}
