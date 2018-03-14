#include "predict.h"
#include "data_preprocess.h"
#include <sstream>
#include <stdio.h>
#include <string>

using namespace std;

string join(char **data, int count) {
    stringstream ss;
    for (int i = 0; i < count; i++) {
        if (data[i] == NULL) break;
        ss << data[i];
    }
    string s = ss.str();
    return s;
}

//你要完成的功能总入口
void predict_server(char *info[MAX_INFO_NUM], char *data[MAX_DATA_NUM],
                    int data_num, char *filename) {
    IdxByFlavor flavor = parse(join(data, MAX_DATA_NUM));
    for (auto vec : flavor) {
        for (auto it : vec.second) {
            printf("flavor = %s date = %s\n", it.flavor.c_str(),
                   it.date.c_str());
        }
    }

    // 需要输出的内容
    char *result_file = (char *)"17\n\n0 8 0 20";

    // 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
    write_result(result_file, filename);
}
