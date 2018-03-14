#if 0

#include "linear_regression.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <random>
#include<cstdlib>
#include<ctime>
#include <cmath>
using namespace std;
#define rd(a,b) (rand()%(b-a+1)+a)

// config for training
int n = 20;
double learn_rate = 1e-3;
double reg = 1e-1;
int num_times = 1000;

double mn = 0;
double var = 0;
vector<double> pred;

vector<Sample> process(string filename) {
    ifstream in(filename);
    vector<int> text;
    int tmp;
    while(in >> tmp) {
        text.push_back(tmp);
    }

    for(auto i:text) {
        mn += i;
    }
    mn = mn / text.size();
    for(auto i:text) {
        var += (i - mn) * (i - mn);
    }
    var = sqrt(var / text.size());
    vector<double> tt;
    for(auto i:text) {
        tt.push_back((i - mn) / var);
    }
    for(int i = tt.size() - n; i <= tt.size() - 1; i++) pred.push_back(tt[i]);
    vector<Sample> ans;
    Sample t;
    for(int i = 0; i < tt.size() - n - 1; i++) {
        vector<int> X;
        for(int j = i; j < i + n; j++) X.push_back(tt[j]);
        t.X = X;
        t.y = tt[i + n];
        ans.push_back(t);
    }
    in.close();
    return ans;
}

void generate(string filename) {
    ofstream out(filename);
    for(int i = 0; i < 400; i++) {
        int tmp = rd(1, 100);
        out << tmp <<endl;
    }
}

int main() {
    LinearRegression *lr = new LinearRegression();
    vector<Sample> ans = process("2.txt");
    // generate("2.txt");
    lr->init(n, ans);
    lr->train(num_times, learn_rate, reg);
    vector<double> ppp = lr->predict(pred, 14);
    for(auto i:ppp) {
        cout << var * i + mn << endl;
    }
    cout << "-------------" << endl;
    cout << mn << " " << var << endl;
    return 0;
}

#endif