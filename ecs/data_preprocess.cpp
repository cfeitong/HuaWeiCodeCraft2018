#include "data_preprocess.h"
#include <sstream>
#include <map>
#include <vector>

using namespace std;


Record parse_line(string line) {
    stringstream ss;
    ss << line;
    Record ret;
    ss >> ret.id;
    ss >> ret.flavor;
    ss >> ret.date;
    ss >> ret.time;
    return ret;
}

IdxByFlavor parse(string content) {
    IdxByFlavor flavors;
    vector<string> text = split(content);
    for (auto line : text) {
        Record record = parse_line(line);
        flavors[record.flavor].push_back(record);
    }
    return flavors;
}

vector<string> split(string text) {
    vector<string> ret;
    
    string line;
    stringstream ss;
    for (ss << text; getline(ss, line); ) {
        ret.push_back(line);
    }
    return ret;
}