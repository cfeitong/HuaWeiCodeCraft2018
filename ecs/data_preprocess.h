#ifndef DATA_PREPROCESS_H_
#define DATA_PREPROCESS_H_

#include <string>
#include <vector>
#include <map>

using namespace std;


struct Record {
    string id;
    string flavor;
    string date;
    string time;
};

typedef map<string, vector<Record>> IdxByFlavor;

Record parse_line(string line);
IdxByFlavor parse(string content);
vector<string> split(string text);

#endif // DATA_PREPROCESS_H_