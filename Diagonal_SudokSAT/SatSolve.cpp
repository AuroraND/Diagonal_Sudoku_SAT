#include "SAT.h"
#include <fstream>
#include <ctime>
using namespace std;

// 决策层变元选择
int NextBranch(Answer *ans)
{
    // 依据var对应branchdecision的值选取判定变元
    int res_var = var_num;
    double max_count = 0;
    double *branch = firstbranch;
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
    if (max_count > 5000)
        for (int i = 1; i < 2 * var_num + 1; i++)
            firstbranch[i] /= 50;
    return (res_var > var_num ? var_num - res_var : res_var);
}

// 判断当前子句能否退化为单子句或冲突子句
int IsUnitClause(int jud_level, Answer *ans, VarTable var_watch[], ClauseTable *&tmp)
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
    {
        while (var != NULL)
        {
            int da = var->data;
            if (da > 0)
                firstbranch[da] += float(var_num) / (clause->cnt * clause->cnt);
            else
                firstbranch[var_num - da] += float(var_num) / (clause->cnt * clause->cnt);
            var = var->nextvar;
        }
        return CONFLICT;
    }
    else if (unknownvar == 1) // 导为单子句则该变元赋为真值继续后续推理
    {
        ans->value[abs(firstvar)] = firstvar / abs(firstvar);                                  // 单子句出现
        ans->branchlevel[abs(firstvar)] = jud_level;                                           // 同一判定级
        known_var++;                                                                           // 已知变元数加一
        tmp = (firstvar > 0 ? var_watch[firstvar].neg->next : var_watch[-firstvar].pos->next); // 该变元的文字邻接表
        return SINGLE;
    }
    else
        return OTHERS;
}

// 依据当前判定变元继续推理
int Deduce(int jud_level, Answer *ans, VarTable var_watch[], ClauseTable *root)
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
            status = IsUnitClause(jud_level, ans, var_watch, tmp);
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
int Contradiction_Analysis(Answer *ans)
{
    jud_level--;
    while (jud_level >= 0 && ans->searched[abs(conflict[jud_level])] == 2)
        jud_level--;
    if (jud_level == -1)
        return 0;
    for (int i = 1; i <= var_num; i++)
        if (ans->branchlevel[i] >= jud_level + 1 && i != abs(conflict[jud_level]))
        {
            ans->branchlevel[i] = 0;
            ans->searched[i] = 0;
            ans->value[i] = UNKNOWN;
            known_var--;
        }
    return conflict[jud_level];
}

int DPLL(Answer *ans, VarTable var_watch[], int op, int firstbranch)
{
    // 结果状态，变元，判定级
    int res, var;
    ClauseTable *ct;
    while (TRUE)
    {
        var = -NextBranch(ans);
        conflict[jud_level++] = var;            // 进入下一分支，决策判定级加一
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
            res = Deduce(jud_level, ans, var_watch, ct);
            if (res == SATISFIABLE) // 满足
                return SATISFIABLE;
            else if (res == CONFLICT) // 当前判定存在冲突
            {
                var = Contradiction_Analysis(ans); // 矛盾分析，返回导致矛盾的根本变元
                // 决策数为0个时依旧存在矛盾则问题不可满足
                if (jud_level == -1)
                    return UNSATISFIABLE;
                // 进入导致矛盾变元的另一分支继续判定
                ans->value[abs(var)] *= -1;
                var *= -1;
                conflict[jud_level++] = var;
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
    InitSat(Head_clause, var_watch, ans);
    Head_clause = LoadCnf(Head_clause, var_watch, ans, path);
    clock_t start_time = clock();
    int res = DPLL(ans, var_watch, 1, 1);
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

// cd D:\myproject\keshe\Diagonal_Sudoku—SAT
// verify5000.exe 1\2.cnf ans.res
// 1      2      3       4     5      6      7     8     9     10     11     12
// 0.034  0.622  0.016   4    0.018   6                               33
