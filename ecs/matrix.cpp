//
// Created by wdk on 2018/3/27.
//

#include "matrix.h"

void copy_mat(Mat &mat1, Mat &mat2) {
    mat1.row = mat2.row;
    mat1.col = mat2.col;
    mat1.mat = mat2.mat;
}

void mat_expand_row(Mat &mat, int num, vector<float> val) {
    mat.row += num;
    vector<float> tmp(mat.col, 0);
    for (int i = 0; i < num; i++) {
        int cnt = i * mat.col;
        for (int j = 0; j < mat.col; j++) tmp[j] = val[cnt + j];
        mat.mat.push_back(tmp);
    }
}

void mat_expand_col(Mat &mat, int num, vector<float> val) {
    int cnt = 0;
    for (int i = 0; i < mat.row; i++) {
        for (int j = 0; j < num; j++) mat.mat[i].push_back(val[cnt++]);
    }
    mat.col += num;
}

void mat_remove_col(Mat &mat, int col_idx) {
    Mat tmp(mat.row, mat.col - 1);
    for (int i = 0; i < mat.row; i++) {
        for (int j = 0; j < mat.col; j++) {
            if (j == col_idx) continue;
            if (j < col_idx) tmp.mat[i][j] = mat.mat[i][j];
            else tmp.mat[i][j - 1] = mat.mat[i][j];
        }
    }
    mat = tmp;
}

void show_mat(Mat &mat) {
    printf("Mat size: row: %d, col: %d\n", mat.row, mat.col);
    for (int i = 0; i < mat.row; i++) {
        for (int j = 0; j < mat.col; j++) {
            printf("%.2f ", mat.mat[i][j]);
        }
        printf("\n");
    }
    puts("");
}