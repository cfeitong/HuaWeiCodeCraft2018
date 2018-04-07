//
// Created by wdk on 2018/3/27.
//
#include "simplex.h"
#include <math.h>
#include <cassert>

#define MAX_VAL_NUM		6144		/* The maximul variable number */

static Mat temp_mat;
static Mat goal_mat;

static int VarName_Map[MAX_VAL_NUM] = {0};
static int BVarOff;
static bool expand_flag;

static int temp_z_col;

float accuracy = 0.01f;
#define	eq(a,b)		( (a-b)<accuracy && (a-b)>-accuracy )
#define	lt(a, b)	((b-a)>accuracy)
#define	le(a, b)	((b-a)>accuracy || ((b-a)>-accuracy && (b-a)<accuracy))
#define isInt(a)	( fabs((fabs(a)-(int)(fabs(a)+0.5))) < accuracy )
#define toInt(a)	(a>0 ? (int)(a+0.5f) : (int)(a-0.5f))

#define RESERVE_SPACE		3


/************************************************************************
	forall b_i>=0, return 0,   otherwise: return 1
************************************************************************/
bool should_expand(Mat &mat) {
    for (int r = 0; r < mat.row; r++) {
        if (mat.mat[r][0] < 0.0)
            return true;
    }
    return false;
}

bool addNonBasicVarName(Mat sf_mat)
{
    if(sf_mat.col >= BVarOff){
        printf("err, non-basic var reserve space is not enough\n");
        return 1;
    }
    VarName_Map[sf_mat.col] = sf_mat.col;

    return 0;
}

void resetSimplexModel(Mat &sf_mat) {
    int i;
    if (should_expand(sf_mat)) {
        if (!expand_flag) {
            addNonBasicVarName(sf_mat);
        }
        else {
            for(i = temp_z_col ; i < sf_mat.col ; i++){
                VarName_Map[i] = VarName_Map[i+1];
            }
            VarName_Map[i] = sf_mat.col;
        }
        vector<float> val(sf_mat.row, 1.0);
        mat_expand_col(sf_mat, 1, val);
        expand_flag = 1;
    }
    else {
        expand_flag = 0;
    }
}

/************************************************************************
Set goal form, calculate the maximum value of goal form.
The last column of mat is constant
************************************************************************/
void setGoalForm(Mat &mat) {
    copy_mat(goal_mat, mat);
    if (expand_flag) {
        vector<float> val(goal_mat.row, -1.0);
        mat_expand_col(goal_mat, 1, val);
    }
}

/************************************************************************
mat: A * x_i <= b_i (x_i >=0)
the first column of matrix stores constant b_i

Slack Form: b_i - A * x_i >= 0
************************************************************************/
void createSlackForm(Mat &mat) {
    for (int r = 0; r < mat.row; r++) {
        for (int c = 1; c < mat.col; c++)
            mat.mat[r][c] = -mat.mat[r][c];
    }
}

/************************************************************************
Find a non-basic variable, which has a positive coefficient
return: -1:doesn't have positive coefficient  otherwise: index of coefficient
************************************************************************/
int findNextNonBasicVar(void)
{
    int i;

    /* the first column is constant */
    for(i = 1 ; i < goal_mat.col ; i++){
        /* ignore z */
        if( expand_flag && VarName_Map[i] == goal_mat.col-1 )
            continue;
        if( goal_mat.mat[0][i] > accuracy )
            return i;
    }

    return -1;
}

int findNextNonBasicVarWithZ(Mat sf_mat, int row) {
    for (int col = 1; col < sf_mat.col; col++) {
        if (lt(sf_mat.mat[row][col], 0.0f)) {
            return col;
        }
    }
    return -1;
}

/************************************************************************
     find most negative b_i to be pivot with z
************************************************************************/
PIVOT findMostNegativePivot(Mat sf_mat) {
    float neg_b;
    PIVOT pivot;

    pivot.bvar = -1;
    pivot.nbvar = sf_mat.col - 1;

    neg_b = -accuracy;
    for (int r = 0; r < sf_mat.row; r++) {
        if (sf_mat.mat[r][0] < neg_b) {
            neg_b = sf_mat.mat[r][0];
            pivot.bvar = r;
        }
    }
    return pivot;
}

PIVOT findPivot(Mat sf_mat, int nbvar) {
    float tight_constrain = 100000000.0f;
    float temp_constrain;
    PIVOT pivot;

    pivot.bvar = -1;	/* -1 means doesn't find a valid pivot */
    pivot.nbvar = nbvar;

    for (int row = 0 ; row < sf_mat.row ; row++){
        if (eq(0.0f, sf_mat.mat[row][nbvar]) || sf_mat.mat[row][nbvar] > 0.0f)
            continue;
        temp_constrain = sf_mat.mat[row][0] / -sf_mat.mat[row][nbvar];
        if (temp_constrain < tight_constrain){
            tight_constrain = temp_constrain;
            pivot.bvar = row;
        }
    }

    return pivot;
}

void swapNameSpace(PIVOT pivot)
{
    int map_index;

    map_index = VarName_Map[pivot.nbvar];
    VarName_Map[pivot.nbvar] = VarName_Map[pivot.bvar+BVarOff];
    VarName_Map[pivot.bvar+BVarOff] = map_index;
}

/************************************************************************
        swap basic var and non-basic var
************************************************************************/
void swapPivot(Mat &sf_mat, PIVOT pivot) {
    float temp = sf_mat.mat[pivot.bvar][pivot.nbvar];
    for (int c = 0; c < sf_mat.col; c++) {
        if (c == pivot.nbvar)
            sf_mat.mat[pivot.bvar][c] = 1.0/temp;
        else
            sf_mat.mat[pivot.bvar][c] /= -temp;
    }
}

void replaceNewBasicVar(Mat &sf_mat, PIVOT pivot) {
    for (int r = 0; r < sf_mat.row; r++) {
        if (r == pivot.bvar) continue;
        // coefbuff[row] = sf_mat->mem[row][pivot.nbvar];
        auto coefbuff = sf_mat.mat[r][pivot.nbvar];
        for (int c = 0; c < sf_mat.col; c++) {
            if (c == pivot.nbvar)
                sf_mat.mat[r][c] = coefbuff * sf_mat.mat[pivot.bvar][c];
            else
                sf_mat.mat[r][c] += coefbuff * sf_mat.mat[pivot.bvar][c];
        }
    }
}
void updateGoalForm(Mat &gf_mat, Mat &sf_mat, PIVOT pivot) {
    float tmp = gf_mat.mat[0][pivot.nbvar];
    for (int c = 0; c < gf_mat.col; c++) {
        if (c == pivot.nbvar)
            gf_mat.mat[0][c] = tmp * sf_mat.mat[pivot.bvar][c];
        else
            gf_mat.mat[0][c] += tmp * sf_mat.mat[pivot.bvar][c];
    }
}

/************************************************************************
swap pivot and reformulate slack form
************************************************************************/
void reformulation(Mat &sf_mat, PIVOT pivot) {
    swapNameSpace(pivot);
    swapPivot(sf_mat, pivot);
    replaceNewBasicVar(sf_mat, pivot);
    updateGoalForm(goal_mat, sf_mat, pivot);
}

int getNonIntIndex(Mat sf_mat, int row_index)
{
    int row, col;

    if(row_index >= sf_mat.row){
        return -1;
    }

    for(row = (int)row_index ; row < sf_mat.row ; row++){
        if( !isInt(sf_mat.mat[row][0]) ){
            for(col = 1 ; col < sf_mat.col ; col++){
                if( !isInt(sf_mat.mat[row][col]) )
                    return row;	/* find a basic var which has non-interger bi and xi */
            }
        }
    }

    return -100;
}

/* add gomory constraint to calculate interger solution */
void addGomoryConstraint(Mat &s_mat,  int row_index)
{
    static int cons_cnt = 1;
    vector<float> gomory_cons;
    gomory_cons.push_back(-(s_mat.mat[row_index][0] - (int)s_mat.mat[row_index][0]));
    for (int c = 1; c < s_mat.col; c++) {
        if (eq(s_mat.mat[row_index][c], 0.0) || isInt(s_mat.mat[row_index][c])) {
            gomory_cons.push_back(0.0);
            continue;
        }
        gomory_cons.push_back(-(s_mat.mat[row_index][c] - (float)ceil(s_mat.mat[row_index][c])));
    }
    assert(gomory_cons.size() == s_mat.col);
    /* add gomory constraint */
    mat_expand_row(s_mat, 1, gomory_cons);
    VarName_Map[s_mat.row-1+BVarOff] = s_mat.row-1+BVarOff;
}

/* remove z column */
int removeZ(Mat &s_mat)
{
    int z_col = -1;
    if(!expand_flag)
        return -1;

    for(z_col = 0 ; z_col < s_mat.col ; z_col++){
        if(VarName_Map[z_col] == s_mat.col-1){
            break;
        }
    }

    if(z_col >= 0){
        mat_remove_col(s_mat, (int)z_col);
        mat_remove_col(goal_mat, (int)z_col);
    }

    return z_col;
}

bool solveSimplex()
{
    bool res = 0;
    int next_nbvar;
    PIVOT pivot;

    /* we should first calculate the min(z), if min(z)=0, then change z to non-basic var */
    if(expand_flag){
        int row_z;

        pivot = findMostNegativePivot(temp_mat);
        row_z = (int)pivot.bvar;

        if(pivot.bvar == -1){
            //printf("NA, findMostNegativePivot\n");
            return 0;
        }
        reformulation(temp_mat, pivot);

        while(1){
            if( eq(temp_mat.mat[row_z][0], 0.0f) ){
                /* min(z)=0 */
                int col;
                bool flag = 0;
                for(col = 1 ; col < temp_mat.col ; col++ ){
                    if( !eq(temp_mat.mat[row_z][col], 0.0f)){
                        pivot.bvar = row_z;
                        pivot.nbvar = col;
                        reformulation(temp_mat, pivot);
                        flag = 1;
                        break;
                    }
                }
                if(flag)
                    break;
                else{
                    printf("some err here\n");
                    while(1);
                }

            }

            next_nbvar = findNextNonBasicVarWithZ(temp_mat, row_z);
            if(next_nbvar == -1){
                //printf("NA, findNextNonBasicVarForZ\n");
                //showResult(goal_mat, 1);
                //showResult(temp_mat, 0);
                return 0;
            }
            pivot = findPivot(temp_mat, (int)next_nbvar);
            reformulation(temp_mat, pivot);
            if(pivot.bvar == row_z){
                //TODO: is it correct?
                /* z becomes non-basic variable now, we can continue */
                break;
            }
        }
    }

    while(1){
        next_nbvar = findNextNonBasicVar();
        if( next_nbvar < 0 ){
            /* non positive coefficient, we get the maximum value */
            res = 1;
            break;
        }

        pivot = findPivot(temp_mat, (int)next_nbvar);
        if(pivot.bvar == -1){
            /* goal formula can be infinite, so doesn't have maximum value */
            //printf("NA, inf result\n");
            res = 0;
            break;
        }else{
            next_nbvar = 1;
        }
        reformulation(temp_mat, pivot);
    }

    return res;
}

bool incise(void)
{
    int nonIntIndex = -1;
    bool res;

    while(1){
        //nonIntIndex = getNonIntIndex(temp_mat, nonIntIndex+1);
        nonIntIndex = getNonIntIndex(temp_mat, 0);
        /* Handle ILP */
        if(nonIntIndex >= 0){
            temp_z_col = removeZ(temp_mat);
            addGomoryConstraint(temp_mat, (int)nonIntIndex);
            resetSimplexModel(temp_mat);
            setGoalForm(goal_mat);

            res = solveSimplex();
            if(!res)	//NA
                return res;
        }else if(nonIntIndex == -1){
            //TODO: is it possible for this situation?
            /* used up all gomory constraints and can't find interger solution */
            //printf("NA, can't find interger solution\n");
            return 0;
        }else{
            /* Get interger soluiton */
            return 1;
        }
    }
}

int getMaxIntValue(void)
{
    return toInt(goal_mat.mat[0][0]);
}

void getIntSolution(vector<int> &solution)
{
    for (int c = 1; c < temp_mat.col - expand_flag; c++) {
        float sol = 0.0;
        for (int r = 0; r < temp_mat.row; r++) {
            if (VarName_Map[r + BVarOff] == c) {
                sol = temp_mat.mat[r][0];
            }
        }
        solution.push_back(toInt(sol));
    }
}

bool initSimplexModel(Mat mat, Mat g_mat)
{
    int i;

    expand_flag = 0;
    copy_mat(temp_mat, mat);

    if(should_expand(temp_mat)){
        expand_flag = 1;
        vector<float> val(temp_mat.row, -1.0);
        mat_expand_col(temp_mat, 1, val);
    }

    //coefbuff = (float*)malloc(mat.row*sizeof(float));

    for(i = 0 ; i < mat.col ; i++){
        VarName_Map[i] = i;
    }
    if(expand_flag){
        VarName_Map[mat.col] = mat.col;
        BVarOff = mat.col + RESERVE_SPACE;
    }else{
        BVarOff = mat.col  + RESERVE_SPACE;	/* basic value offset of VarName array */
    }
    for(i = 0 ; i < mat.row ; i++){
        VarName_Map[i+BVarOff] = i+BVarOff;
    }

    setGoalForm(g_mat);
    createSlackForm(temp_mat);

    return true;
}


bool SimplexRun()
{
    bool res = 0;

    res = solveSimplex();

    if(res){
        res = incise();
        // dispIntSolution(temp_mat, SHOW_RESULT);
    }

    return res;
}

