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
    int time;
};

typedef map<string, vector<Record>> IdxByFlavor;
typedef map<int, vector<Record>> IdxByDate;
typedef map<string, vector<double>> DataByFlavor;
typedef map<string, vector<Sample>> SampleByFlavor;

class RecordSet {
  public:
    RecordSet(const vector<Record> &records);
    ~RecordSet() = default;
    RecordSet(const RecordSet &o) = default;
    RecordSet(RecordSet &&o) = default;

    vector<Sample> to_samples(const string &flavor);
    vector<double> to_data(string flavor);
    vector<Record> at_date(int day);

  private:
    IdxByDate by_date;
    IdxByFlavor by_flavor;
    DataByFlavor data_flavor;
    vector<Record> records;
};

Record parse_line(string line);
vector<Record> parse_records(string content);
vector<string> split(string text);

#endif // DATA_PREPROCESS_H_