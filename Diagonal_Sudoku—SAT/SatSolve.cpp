#include "SAT.h"
#include <fstream>
#include <ctime>
using namespace std;

// 决策层变元选择
int NextBranch(int branchdecision[], Answer *ans)
{
    // 决策层加一
    branch_num++;
    // 依据var对应branchdecision的值选取判定变元
    int res_var = var_num, max_count = 0;
    int *branch = (branch_num == 1 ? firstbranch : branchdecision);
    for (int i = 1; i < 2 * var_num + 1; i++)
    {
        // 已确定的变元无需判定直接跳过
        if (i <= var_num && ans->value[i] != UNKNOWN)
            continue;
        if (i > var_num && ans->value[i - var_num] != UNKNOWN)
            continue;
        if (max_count < *(branch + i))
        {
            res_var = i;
            max_count = *(branch + i);
        }
    }
    return (res_var > var_num ? var_num - res_var : res_var);
}

// 判断当前子句能否退化为单子句或冲突子句
int IsUnitClause(int jud_level, Answer *ans, VarTable var_watch[], int branchdecision[], ClauseTable *&tmp)
{
    // 子句中未知变元个数及第一个未知变元
    int unknownvar = 0, firstvar = 0;
    // 子句是否为已满足
    int satisfiable = FALSE;
    // 当前子句
    Clause *clause = tmp->headclause;
    VarList *var = clause->headvar->nextvar;
    while (var != NULL)
    {
        int da = var->data;
        // 存在真值文字该子句已满足
        if (ans->value[abs(da)] * da > 0)
        {
            satisfiable = TRUE;
            break;
        }
        // 分支决策计数
        if (da > 0)
            branchdecision[da]++;
        else
            branchdecision[var_num - da]++;
        // 统计未知变元
        if (ans->value[abs(da)] == UNKNOWN)
        {
            unknownvar++;
            // 赋值第一个未知变元
            if (unknownvar == 1)
                firstvar = da;
        }
        var = var->nextvar;
    }
    if (satisfiable == TRUE)
        return SATISFIABLE;
    else if (!unknownvar) // 变元均已知子句却未满足则该子句存在冲突无法满足
        return CONFLICT;
    else if (unknownvar == 1) // 导为单子句则该变元赋为真值继续后续推理
    {
        // cout << firstvar << '\n';
        ans->value[abs(firstvar)] = firstvar / abs(firstvar);
        ans->singleclause[abs(firstvar)] = TRUE;                                               // 单子句出现
        ans->branchlevel[abs(firstvar)] = jud_level;                                           // 同一判定级
        known_var++;                                                                           // 已知变元数加一
        tmp = (firstvar > 0 ? var_watch[firstvar].neg->next : var_watch[-firstvar].pos->next); // 该变元的文字邻接表
        return SINGLE;
    }
    else
        return OTHERS;
}

// 依据当前判定变元继续推理
int Deduce(int jud_level, Answer *ans, VarTable var_watch[], int branchdecision[], ClauseTable *root)
{
    // 栈顶
    int top = 0;
    // 构建栈存储由当前判定变元引起的新一批单子句的迹
    ClauseTable *stack[MaxVarNum], *tmp = root;
    // 判定变元的子句邻接表入栈
    stack[top++] = tmp;
    // 推理开始
    while (top)
    {
        tmp = stack[top - 1]; // 取栈顶子句邻接表
        int status = SINGLE;  // 子句判断当前情况下是否能变为单子句
        while (status == SINGLE && tmp != NULL)
        {
            status = IsUnitClause(jud_level, ans, var_watch, branchdecision, tmp);
            if (status == SINGLE) // 单子句变元子句邻接表入栈
                stack[top++] = tmp;
        }
        if (status == CONFLICT) // 出现矛盾
            return CONFLICT;
        // 当下子句无法导出单子句出栈利用子句邻接表替换为下一子句
        tmp = stack[--top];
        if (tmp->next != NULL)
            stack[top++] = tmp->next;
    }
    if (known_var < var_num)
        return OTHERS;
    else
        return SATISFIABLE;
}

// 矛盾分析，回溯到引发矛盾的判定层走另一分支
int Contradiction_Analysis(int &jud_level, int var, Answer *ans)
{
    int da = abs(var);
    while (jud_level)
    {
        // 由当前变元决策层推断出的变元全部重置
        for (int i = 1; i <= var_num; i++)
            if (i != da && ans->branchlevel[i] == jud_level)
            {
                ans->branchlevel[i] = 0;
                ans->searched[i] = 0;
                ans->singleclause[i] = FALSE;
                ans->value[i] = UNKNOWN;
                known_var--;
            }
        // 两分支均搜索过则回溯到上一决策层
        if (ans->searched[da] == 2)
        {
            jud_level--;
            if (!jud_level)
                break;
            ans->branchlevel[da] = 0;
            ans->searched[da] = 0;
            ans->singleclause[da] = FALSE;
            ans->value[da] = UNKNOWN;
            known_var--;
            // 找该决策层的判定变元
            for (int i = 1; i <= var_num; i++)
                if (ans->branchlevel[i] == jud_level && ans->singleclause[i] == FALSE)
                {
                    da = i;
                    break;
                }
        }
        else // 搜索另一分支
            break;
    }
    return da;
}

int DPLL(Answer *ans, VarTable var_watch[], int branchdecision[], int op, int firstbranch)
{
    // 结果状态，变元，判定级
    int res, var, jud_level = 0;
    ClauseTable *ct;
    // int n = 1000;
    while (TRUE)
    {
        var = NextBranch(branchdecision, ans);
        for (int i = 1; i <= var_num; i++)
            branchdecision[i] = 0;
        jud_level++;                            // 进入下一分支，决策判定级加一
        ans->value[abs(var)] = var / abs(var);  // 该变元判定为var
        ans->branchlevel[abs(var)] = jud_level; // 当前判定级
        ans->searched[abs(var)]++;              // 该变元的分支搜索数
        known_var++;                            // 已知变元数
        // cout << (double(known_var) / var_num) * 100 << "%" << '\n';
        while (TRUE) //  利用当前判定变元继续推理
        {
            // 搜索与当前判定值相悖的子句
            ct = (var > 0 ? var_watch[var].neg->next : var_watch[-var].pos->next);
            // cout << var << '\n';
            res = Deduce(jud_level, ans, var_watch, branchdecision, ct);
            if (res == SATISFIABLE) // 满足
                return SATISFIABLE;
            else if (res == CONFLICT) // 当前判定存在冲突
            {
                var = Contradiction_Analysis(jud_level, var, ans); // 矛盾分析，返回导致矛盾的根本变元
                // cout << var << ' ' << ans->searched[abs(var)] << ' ' << jud_level << '\n';
                // 判定级为0是依旧存在矛盾则问题不可满足
                if (!jud_level)
                    return UNSATISFIABLE;
                // 进入导致矛盾变元的另一分支继续判定
                ans->value[abs(var)] *= -1;
                if (ans->value[abs(var)] < 0)
                    var *= -1;
                // 搜索分支数加一
                ans->searched[abs(var)]++;
            }
            else if (res == OTHERS) // 条件不足进入下一层决策
                break;
        }
    }
    // 运行错误
    return ERROR;
}

void print_ans(Answer *ans)
{
    for (int i = 1; i <= var_num; i++)
        cout << ans->value[i] * i << ' ';
}

void Sat(string path)
{
    Clause *Head_clause = NULL;
    Answer *ans = new Answer;
    VarTable var_watch[MaxVarNum];
    int branchdecision[MaxVarNum];
    InitSat(Head_clause, var_watch, ans, branchdecision);
    Head_clause = LoadCnf(Head_clause, var_watch, ans, path);
    clock_t start_time = clock();
    int res = DPLL(ans, var_watch, branchdecision, 1, 1);
    clock_t end_time = clock();
    string path1 = ".\\ans.res";
    ofstream outfile(path1);
    if (!outfile.is_open())
    {
        cout << "文件打开失败" << '\n';
        return;
    }
    cout << "s ";
    outfile << "s ";
    if (res == SATISFIABLE)
    {
        cout << 1 << '\n';
        outfile << 1 << '\n';
    }
    else
    {
        cout << 0 << '\n';
        outfile << 0 << '\n';
    }
    cout << "v ";
    outfile << "v ";
    if (res == SATISFIABLE)
    {
        print_ans(ans);
        for (int i = 1; i <= var_num; i++)
            outfile << ans->value[i] * i << ' ';
    }
    else
    {
        cout << "unsatisfiable";
        outfile << "unsatisfiable";
    }
    cout << '\n';
    outfile << '\n';
    cout << "t " << (end_time - start_time) << '\n';
    outfile << "t " << (end_time - start_time) << '\n';
}

// g++ main.cpp SatSolve.cpp CnfPaster.cpp X-sudoku.cpp -o res

// verify5000.exe D:\myproject\keshe\1\1.cnf D:\myproject\keshe\1\x.res
// 1      2      3       4     5      6      7     8     9     10     11     12
// 0.024  0.037  0.006   20     0.005  1.5                             97
