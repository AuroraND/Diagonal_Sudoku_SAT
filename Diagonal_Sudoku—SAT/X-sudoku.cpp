#include <fstream>
#include "SAT.h"
#include <ctime>
#include <algorithm>
#include <random>
using namespace std;

void CreateSudokuRule()
{
    string path = ".\\sudoku_rule.cnf";
    ofstream outfile(path);
    if (!outfile.is_open())
    {
        cout << "文件打开失败" << '\n';
        return;
    }
    // 编写数独规则
    outfile << "p cnf 729 10953" << '\n';
    // 行限制
    for (int i = 0; i < 9; i++)
        for (int k = 1; k < 10; k++)
            for (int j = 0; j < 9; j++)
                for (int n = j + 1; n < 9; n++)
                    outfile << -(i * 81 + j * 9 + k) << ' ' << -(i * 81 + n * 9 + k) << ' ' << 0 << '\n';

    // 列限制
    for (int j = 0; j < 9; j++)
        for (int k = 1; k < 10; k++)
            for (int i = 0; i < 9; i++)
                for (int n = i + 1; n < 9; n++)
                    outfile << -(i * 81 + j * 9 + k) << ' ' << -(n * 81 + j * 9 + k) << ' ' << 0 << '\n';

    // 块限制
    for (int k = 1; k < 10; k++)
        for (int c = 0; c < 3; c++)
            for (int n = 0; n < 3; n++)
            {
                int tmp = c * 81 * 3 + n * 9 * 3 + k;
                for (int t = 0; t < 3; t++)
                    for (int s = 0; s < 3; s++)
                        outfile << tmp + t * 81 + s * 9 << ' ';
                outfile << 0 << '\n';
                for (int t = 0; t < 2; t++)
                    for (int s = 0; s < 3; s++)
                        for (int i = t + 1; i < 3; i++)
                            for (int j = 0; j < 3; j++)
                                if (s != j)
                                    outfile << -(tmp + t * 81 + s * 9) << ' ' << -(tmp + i * 81 + j * 9) << ' ' << 0 << '\n';
            }

    // 对角线限制
    for (int k = 1; k < 10; k++)
        for (int i = 0; i < 9; i++)
            for (int j = i + 1; j < 9; j++)
            {
                outfile << -(i * 81 + i * 9 + k) << ' ' << -(j * 81 + j * 9 + k) << ' ' << 0 << '\n';
                outfile << -(i * 81 + (8 - i) * 9 + k) << ' ' << -(j * 81 + (8 - j) * 9 + k) << ' ' << 0 << '\n';
            }

    for (int k = 1; k < 10; k++)
    {
        for (int i = 0; i < 9; i++)
            outfile << i * 81 + i * 9 + k << ' ';
        outfile << 0 << '\n';
        for (int i = 0; i < 9; i++)
            outfile << i * 81 + (8 - i) * 9 + k << ' ';
        outfile << 0 << '\n';
    }

    // 单点限制
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            for (int k = 1; k < 10; k++)
                for (int n = k + 1; n < 10; n++)
                    outfile << -(i * 81 + j * 9 + k) << ' ' << -(i * 81 + j * 9 + n) << ' ' << 0 << '\n';
    outfile.close();
}

int isvalid(int sud[][10], int row, int col, int value)
{
    // 块儿不满足
    int x = (row > 6 ? 7 : (row > 3 ? 4 : 1));
    int y = (col > 6 ? 7 : (col > 3 ? 4 : 1));
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (sud[x + i][y + j] == value)
                return FALSE;
    // 行列不满足
    for (int i = 1; i < 10; i++)
        if (sud[row][i] == value || sud[i][col] == value)
            return FALSE;
    // 对角线不满足
    if (row == col)
        for (int i = 1; i < 10; i++)
            if (sud[i][i] == value)
                return FALSE;
    if (row + col == 10)
        for (int i = 1; i < 10; i++)
            if (sud[i][10 - i] == value)
                return FALSE;
    // 满足限制
    return TRUE;
}

// 挖洞法生成数独初盘
void DigHole(Answer *ans, int que[][10], VarTable var_watch[], int branchdecision[])
{
    int hole[90];
    // 随机数生成器
    mt19937 g(rand());
    for (int i = 1; i < 82; i++)
        hole[i] = i;
    shuffle(hole + 1, hole + 82, g);
    for (int i = 1; i <= 81; i++)
    {
        int row = (hole[i] - 1) / 9 + 1;
        int col = (!(hole[i] % 9) ? 9 : hole[i] % 9);
        int value = que[row][col], f = 1;
        for (int j = 1; j < 10; j++)
        {
            if (j == value)
                continue;
            que[row][col] = 0;
            if (isvalid(que, row, col, j) == TRUE)
            {
                f = 0;
                break;
            }
        }
        if (!f)
            que[row][col] = value;
    }
}

void print_sudoku(int sud[][10])
{
    for (int i = 1; i < 10; i++)
    {
        for (int j = 1; j < 10; j++)
            cout << sud[i][j] << ' ';
        cout << '\n';
    }
}

// 生成数独终盘
void CreateSudoku(int sud[][10], int que[][10])
{
    // 读入数独规则
    string path = ".\\sudoku_rule.cnf";
    Clause *Head_clause = NULL;
    Answer *ans = new Answer;
    VarTable var_watch[MaxVarNum];
    int branchdecision[MaxVarNum];
    // 采用当前时间作为随机数生成依据
    unsigned int seed = time(nullptr) % 100000000;
    // 随机数种子
    srand(seed);
    do
    {
        InitSat(Head_clause, var_watch, ans, branchdecision);
        Head_clause = LoadCnf(Head_clause, var_watch, ans, path);
        for (int i = 1; i < 10; i++)
            for (int j = 1; j < 10; j++)
                sud[i][j] = 0;
        // 随机填入一个数字
        int row = rand() % 9 + 1;
        int col = rand() % 9 + 1;
        int value = rand() % 9 + 1;
        int var = (row - 1) * 81 + (col - 1) * 9 + value;
        ans->value[var] = 1;
        known_var++;
    } while (DPLL(ans, var_watch, branchdecision, 1, 1) != SATISFIABLE);
    // print_ans(ans, var_num);
    // 将ans转换为数独
    for (int i = 1; i <= 729; i++)
    {
        if (ans->value[i] > 0)
        {
            int x, y, z;
            x = (i - 1) / 81 + 1;
            if (i % 81)
                y = (i % 81 - 1) / 9 + 1;
            else
                y = 9;
            z = (i % 9 ? i % 9 : 9);
            sud[x][y] = z;
        }
    }
    for (int i = 1; i < 10; i++)
        for (int j = 1; j < 10; j++)
            que[i][j] = sud[i][j];
    DigHole(ans, que, var_watch, branchdecision);
}

// g++ main.cpp SatSolve.cpp CnfPaster.cpp X-sudoku.cpp -o res