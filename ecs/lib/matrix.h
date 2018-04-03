//
// Created by 12996 on 2018/3/27.
//

#ifndef __MATRIX__
#define __MATRIX__

#include <cstdio>
#include <vector>
using namespace std;

struct Mat {
    vector<vector<float>> mat;
    int row, col;
    Mat() {}
    Mat(int r, int c) : mat(r, vector<float>(c, 0)), row(r), col(c) {}
};

void copy_mat(Mat &mat1, Mat &mat2);
void mat_expand_row(Mat &mat, int num, vector<float> val);
void mat_expand_col(Mat &mat, int num, vector<float> val);
void mat_remove_col(Mat &mat, int col_idx);
void show_mat(Mat &mat);

#endif
