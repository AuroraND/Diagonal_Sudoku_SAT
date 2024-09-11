#include <iostream>
using namespace std;
#ifndef SAT_H_INCLUDED
#define SAT_H_INCLUDED

#define MaxVarNum 10000
// 子句状态
#define ERROR -2
#define CONFLICT -1
#define SATISFIABLE 0
#define UNSATISFIABLE 1
#define OTHERS 2
#define SINGLE 3
// 变元状态
#define TRUE 1
#define FALSE -1
#define UNKNOWN 0
#define NONE 2

// 变元链表
typedef struct VarList
{
    int data;
    struct VarList *nextvar;
} VarList;

// 子句链表
typedef struct Clause
{
    VarList *headvar;
    struct Clause *nextclause;
    int cnt;
} Clause;

// 子句邻接表
typedef struct ClauseTable
{
    Clause *headclause;
    struct ClauseTable *next;
} ClauseTable;

// 正负文字邻接表
typedef struct VarTable
{
    ClauseTable *pos;
    ClauseTable *neg;
} VarTable;

// 求解标记及答案
typedef struct Answer
{
    int branchlevel[MaxVarNum];  // 决策树高度
    int value[MaxVarNum];        // 变元真假值
    int searched[MaxVarNum];     // 搜索过的分支个数
    int singleclause[MaxVarNum]; // 标记是否存在该变元单子句
} Answer;

// 函数

void Sat(string path);

void print_ans(Answer *ans);

int DPLL(Answer *ans, VarTable var_watch[], int op, int firstbranch);

int Contradiction_Analysis(int &jud_level, int var, Answer *ans);

int Deduce(int jud_level, Answer *ans, VarTable var_watch[], ClauseTable *root);

int IsUnitClause(int jud_level, Answer *ans, VarTable var_watch[], ClauseTable *&tmp);

int NextBranch(Answer *ans);

Clause *LoadCnf(Clause *Head_clause, VarTable war_watch[], Answer *ans, string path);

void CreateClause(Clause *clause, VarTable war_watch[], Answer *ans, string s, int &varclausenum);

void InitSat(Clause *Head_clause, VarTable var_watch[], Answer *ans);

// 总变元数
extern int var_num;
// 总子句数
extern int clause_num;
// 已确定变元数
extern int known_var;
// 分支数
extern int jud_level;
// 冲突决策栈
extern int conflict[MaxVarNum];
//
extern double firstbranch[MaxVarNum];

#endif