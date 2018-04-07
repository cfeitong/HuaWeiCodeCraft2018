//
// Created by wdk on 2018/3/27.
//

#ifndef __SIMPLEX__
#define __SIMPLEX__

#include "matrix.h"

typedef struct {
    int bvar;	//basic variable: yi
    int nbvar;	//non-basic variable: xi
}PIVOT;

bool initSimplexModel(Mat mat, Mat g_mat);
bool SimplexRun();
int getMaxIntValue();
void getIntSolution(vector<int> &solution);

#endif
