#include "constant.h"
#include <cstdio>
#include <string>
#include <vector>
#include <map>

using namespace std;
vector<double> flavor1{-0.0842955, -0.0701982, 0.0332979, 0.288053, -0.0297573};
vector<double> flavor2{-0.0900975, -0.0301779, 0.207661, 0.266209, 0.040215};
vector<double> flavor3{-0.133344, 0.0263402, 0.092202, 0.263333, 0.0263292};
vector<double> flavor4{-0.0936547, -0.0743981, 0.0523058, 0.226823, 0.0622143};
vector<double> flavor5{-0.126623, 0.0483097, 0.156151, 0.230834, 0.102412};
vector<double> flavor6{-0.173817, -0.0383588, 0.147104, 0.273503, 0.1475};
vector<double> flavor7{-0.0176509, -0.0560981, 0.0710776, 0.344972, 0.0956744};
vector<double> flavor8{-0.121556, -0.0697948, 0.299238, 0.205416, 0.0150809};
vector<double> flavor9{-0.0460439, -0.0506089, 0.0617733, 0.360071, 0.0640445};
vector<double> flavor10{-0.107495, -0.0118375, 0.104704, 0.23425, -0.0356902};
vector<double> flavor11{-0.100399, -0.0793481, 0.165562, 0.363066, 0.064114};
vector<double> flavor12{-0.0946431, -0.058043, 0.0754983, 0.305291, 0.0323331};
vector<double> flavor13{-0.173706, -0.103802, -0.089649, 0.804826, 0.0401526};
vector<double> flavor14{-0.254909, 0.0187683, 0.18708, 0.419047, 0.0854988};
vector<double> flavor15{-0.0810563, 0.0569241, 0.0493709, 0.215964, -0.029588};

vector<double> get_pretrained(string flavor) {
    map<string, vector<double>> mp;
    mp["flavor1"] = flavor1;
    mp["flavor2"] = flavor2;
    mp["flavor3"] = flavor3;
    mp["flavor4"] = flavor4;
    mp["flavor5"] = flavor5;
    mp["flavor6"] = flavor6;
    mp["flavor7"] = flavor7;
    mp["flavor8"] = flavor8;
    mp["flavor9"] = flavor9;
    mp["flavor10"] = flavor10;
    mp["flavor11"] = flavor11;
    mp["flavor12"] = flavor12;
    mp["flavor13"] = flavor13;
    mp["flavor14"] = flavor14;
    mp["flavor15"] = flavor15;
    return mp[flavor];
}

int flavorid(const string &flavor) {
    int id;
    sscanf(flavor.c_str(), "flavor%d", &id);
    return id;
}