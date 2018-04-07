#include "lib_io.h"
#include "lib_time.h"
#include <cassert>
#include <cerrno>
#include <csignal>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/timeb.h>
#include <ctime>
#include <unistd.h>
#include <cmath>

using namespace std;

#define MAX_LINE_LEN 55000

#define INLINE static __inline
#ifdef _DEBUG
#define PRINT printf
#else
#define PRINT(...)
#endif

static char s[100];
static char final_result[10000];

INLINE void write_file(const bool cover, const char *const buff,
                       const char *const filename);

Info::Info(char **info) {
    int line = 0;
    int cpu, mem, disk;
    sscanf(info[line++], "%d%d%d", &cpu, &mem, &disk);
    int needed;
    line++;
    sscanf(info[line++], "%d", &needed);
    vector<string> tar;
    for (int i = 0; i < needed; i++) {
        sscanf(info[line++], "%s", s);
        tar.emplace_back(s);
    }
    line++;
    sscanf(info[line++], "%s", s);
    string type(s);
    line++;
    int sy, sm, sd;
    sscanf(info[line++], "%d-%d-%d", &sy, &sm, &sd);
    int ey, em, ed;
    sscanf(info[line++], "%d-%d-%d", &ey, &em, &ed);
    int start = to_days(sy, sm, sd);
    int end = to_days(ey, em, ed);
    int len = end - start;
    mem *= 1024;

    this->days = len;
    this->start_date = start;
    this->end_date = end;
    this->cpu_lim = cpu;
    this->mem_lim = mem;
    this->targets = tar;
    this->opt_type = type;
}

Outputor::Outputor(Allocator &alloc, const Info &meta) {
    stringstream ss;
    int total = 0;
    for (const string &flavor : meta.targets) {
        int cnt = alloc.count_vir(flavor);
        total += cnt;
    }
    ss << total << "\n";
    for (const string &flavor : meta.targets) {
        int cnt = alloc.count_vir(flavor);
        ss << flavor << " " << cnt << "\n";
    }
    ss << "\n";

    int phy_count = alloc.count_phy();
    ss << phy_count << "\n";
    for (int i = 1; i <= phy_count; i++) {
        ss << i;
        for (const string &flavor : meta.targets) {
            int cnt = alloc.count(i, flavor);
            ss << " " << flavor << " " << cnt;
        }
        ss << "\n";
    }
    this->result = ss.str();
}

char *Outputor::get_output() {
    strcpy(final_result, this->result.c_str());
    return final_result;
}

int to_days(int year, int month, int day) {
    struct tm a = {0, 0, 0, 0, 1, 100};
    struct tm b = {0, 0, 0, day, month, year - 1900};
    time_t x = mktime(&a);
    time_t y = mktime(&b);
    long difference = 0;
    if (x != (time_t)(-1) && y != (time_t)(-1)) {
        difference = lround(difftime(y, x) / (60 * 60 * 24));
    } else {
        exit(-1);
    }
    return difference;
}

void print_time(const char *head) {
#ifdef _DEBUG
    struct timeb rawtime;
    struct tm *timeinfo;
    ftime(&rawtime);
    timeinfo = localtime(&rawtime.time);

    static int ms = rawtime.millitm;
    static unsigned long s = rawtime.time;
    int out_ms = rawtime.millitm - ms;
    unsigned long out_s = rawtime.time - s;
    ms = rawtime.millitm;
    s = rawtime.time;

    if (out_ms < 0) {
        out_ms += 1000;
        out_s -= 1;
    }
    printf("%s date/time is: %s \tused time is %lu s %d ms.\n", head,
           asctime(timeinfo), out_s, out_ms);
#endif
}

int read_file(char **const buff, const unsigned int spec,
              const char *const filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        PRINT("Fail to open file %s, %s.\n", filename, strerror(errno));
        return 0;
    }
    PRINT("Open file %s OK.\n", filename);

    char line[MAX_LINE_LEN + 2];
    unsigned int cnt = 0;
    while ((cnt < spec) && !feof(fp)) {
        line[0] = 0;
        if (fgets(line, MAX_LINE_LEN + 2, fp) == NULL)
            continue;
        if (line[0] == 0)
            continue;
        buff[cnt] = (char *)malloc(MAX_LINE_LEN + 2);
        strncpy(buff[cnt], line, MAX_LINE_LEN + 2 - 1);
        buff[cnt][MAX_LINE_LEN + 1] = 0;
        cnt++;
    }
    fclose(fp);
    PRINT("There are %d lines in file %s.\n", cnt, filename);

    return cnt;
}

void write_result(const char *const buff, const char *const filename) {
    // 以覆盖的方式写入
    write_file(1, buff, filename);
}

void release_buff(char **const buff, const int valid_item_num) {
    for (int i = 0; i < valid_item_num; i++)
        free(buff[i]);
}

INLINE void write_file(const bool cover, const char *const buff,
                       const char *const filename) {
    if (buff == NULL)
        return;

    const char *write_type = cover ? "w" : "a"; // 1:覆盖写文件，0:追加写文件
    FILE *fp = fopen(filename, write_type);
    if (fp == NULL) {
        PRINT("Fail to open file %s, %s.\n", filename, strerror(errno));
        return;
    }
    PRINT("Open file %s OK.\n", filename);
    fputs(buff, fp);
    fputs("\n", fp);
    fclose(fp);
}
