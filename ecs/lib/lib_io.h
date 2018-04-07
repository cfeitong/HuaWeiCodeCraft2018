#ifndef __LIB_IO_H__
#define __LIB_IO_H__

#define MAX_INFO_NUM 50
#define MAX_DATA_NUM 50000

#include "allocation.h"
#include <string>
#include <vector>
using namespace std;

//读取文件并按行输出到buff。
// buff为一个指针数组，每一个元素是一个字符指针，对应文件中一行的内容。
// spec为允许解析的最大行数。
extern int read_file(char **const buff, const unsigned int spec,
                     const char *const filename);

//将result缓冲区中的内容写入文件，写入方式为覆盖写入
extern void write_result(const char *const buff, const char *const filename);

//释放读文件的缓冲区
extern void release_buff(char **const buff, const int valid_item_num);

struct Info {
    int days;               // how many day to predict
    int start_date;
    int end_date;
    int cpu_lim;            // cpu per physic machine
    int mem_lim;            // memory per physic machine
    vector<string> targets; // target flavor
    string opt_type;        // CPU or MEM to optimize

    Info(char **info);
};

class Outputor {
  public:
    Outputor(Allocator &alloc, const Info &meta);
    char *get_output();

  private:
    string result;
};

#endif
