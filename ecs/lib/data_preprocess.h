#ifndef DATA_PREPROCESS_H_
#define DATA_PREPROCESS_H_

#include <map>
#include <string>
#include <vector>

using namespace std;

struct Sample {
    vector<double> X;
    double y;
};

struct Record {
    string id;
    string flavor;
    string date;
    string time;
};

typedef map<string, vector<Record>> IdxByFlavor;
typedef map<string, vector<Record>> IdxByDate;

class RecordSet {
  public:
    RecordSet(const vector<Record> &records);
    ~RecordSet() = default;
    RecordSet(const RecordSet &o) = default;
    RecordSet(RecordSet &&o) = default;

    int cpu_required(string date);
    int mem_required(string date);
    vector<Sample> to_samples();

  private:
    IdxByDate by_date;
    IdxByFlavor by_flavor;
    vector<Record> records;
};

Record parse_line(string line);
vector<Record> parse(string content);
vector<string> split(string text);

#endif // DATA_PREPROCESS_H_