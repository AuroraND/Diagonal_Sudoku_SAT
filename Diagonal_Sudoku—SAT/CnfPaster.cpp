#include "SAT.h"
#include <fstream>
#include <ctime>
using namespace std;

// 文件载入
void InitSat(Clause *Head_clause, VarTable var_watch[], Answer *ans, int branchdecision[])
{
    Clause *cur_clause = Head_clause, *nex_clause;
    VarList *cur_var, *nex_var;
    // 子句链表初始化
    while (cur_clause != NULL)
    {
        nex_clause = cur_clause->nextclause;
        cur_var = nex_clause->headvar;
        // 变元链表初始化
        while (cur_var != NULL)
        {
            nex_var = cur_var->nextvar;
            free(cur_var);
            cur_var = nex_var;
        }
        free(cur_clause);
        cur_clause = nex_clause;
    }
    Head_clause = NULL;
    // 基本变量初始化
    var_num = known_var = branch_num = 0;
    for (int i = 0; i < MaxVarNum; i++)
    {
        // 解初始化
        ans->value[i] = NONE;
        ans->branchlevel[i] = 0;
        ans->searched[i] = 0;
        ans->singleclause[i] = FALSE;
        // 正负文字邻接表初始化
        var_watch[i].neg = new ClauseTable;
        var_watch[i].neg->next = NULL;
        var_watch[i].pos = new ClauseTable;
        var_watch[i].pos->next = NULL;
    }
    // 分支决策计数器初始化
    for (int i = 0; i < MaxVarNum; i++)
        branchdecision[i] = 0;
}

// 创建s中所有变元组成的子句
void CreateClause(Clause *clause, VarTable war_watch[], Answer *ans, string s, int &varclausenum)
{
    clause->headvar = new VarList;
    VarList *cur_var = clause->headvar;
    cur_var->nextvar = NULL;
    int num = 0, f = 0;
    for (int i = 0; i < s.size(); i++)
    {
        if (isdigit(s[i]))
            num = num * 10 + s[i] - '0';
        else if (s[i] == '-')
            f = 1;
        else if (s[i] == ' ')
        {
            if (ans->value[num] == NONE)
                ans->value[num] = UNKNOWN;
            ClauseTable *wa;
            // 初始决策计数及正负文字邻接表赋值
            if (f)
            {
                wa = war_watch[num].neg;
                firstbranch[var_num + num]++;
            }
            else
            {
                wa = war_watch[num].pos;
                firstbranch[num]++;
            }
            while (wa->next != NULL)
                wa = wa->next;
            wa->next = new ClauseTable;
            wa = wa->next;
            wa->headclause = clause;
            wa->next = NULL;
            // 子句链表赋值
            cur_var->nextvar = new VarList;
            cur_var = cur_var->nextvar;
            cur_var->data = (!f ? num : -num);
            cur_var->nextvar = NULL;
            num = 0, f = 0;
            varclausenum++;
        }
        else if (s[i] == '0')
            break;
    }
}

Clause *LoadCnf(Clause *Head_clause, VarTable war_watch[], Answer *ans, string path)
{
    ifstream infile(path);
    if (!infile.is_open())
    {
        cout << "文件打开失败" << '\n';
        exit(1);
    }
    // 读取注释内容
    char c;
    string line;
    infile >> c;
    while (c == 'c')
    {
        getline(infile, line);
        infile >> c;
    }
    // 读取变元数与子句数
    infile >> line >> var_num >> clause_num;
    getline(infile, line);
    // 读取各子句变元
    Clause *tmp, *tmpclause = NULL;
    while (getline(infile, line))
    {
        int numclausevar = 0;
        tmp = new Clause;
        tmp->nextclause = NULL;
        CreateClause(tmp, war_watch, ans, line, numclausevar);
        if (numclausevar == 1)
        {
            int da = tmp->headvar->nextvar->data;
            ans->value[abs(da)] = da / abs(da);
            known_var++;
        }
        else if (Head_clause == NULL)
            Head_clause = tmpclause = tmp;
        else
        {
            tmpclause->nextclause = tmp;
            tmpclause = tmpclause->nextclause;
        }
    }
    infile.close();
    return Head_clause;
}