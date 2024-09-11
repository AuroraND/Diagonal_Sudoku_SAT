#include "SAT.h"
#include "Sud.h"
#include "CnfPaster.cpp"
#include "X-sudoku.cpp"
#include "SatSolve.cpp"
using namespace std;

// 总变元数
int var_num;
// 总子句数
int clause_num;
// 已确定变元数
int known_var;
// 分支数
int jud_level;
// 冲突决策栈
int conflict[MaxVarNum];
//
double firstbranch[MaxVarNum];

int main()
{
    int op = 1;
    while (op)
    {
        system("cls");
        cout << '\n';
        cout << "             Main Menu              " << '\n';
        cout << "************************************" << '\n';
        cout << "*                                  *" << '\n';
        cout << "*    1.Sudoku   2.SAT   0.Exit     *" << '\n';
        cout << "*                                  *" << '\n';
        cout << "************************************" << '\n';
        cout << " Please choose your operation[0--2]:" << '\n';
        cin >> op;
        getchar();
        string path;
        switch (op)
        {
        case 1:
            // 生成数独规则
            // CreateSudokuRule();
            int sud[10][10];
            int que[10][10];
            for (int i = 0; i < 10; i++)
                for (int j = 0; j < 10; j++)
                    sud[i][j] = que[i][j] = 0;
            while (op)
            {
                system("cls");
                cout << '\n';
                cout << "                    Suduku Game                    " << '\n';
                cout << "***************************************************" << '\n';
                cout << "*                                                 *" << '\n';
                cout << "*    1.New_Sudoku_Question   2.Sudoku_Answer      *" << '\n';
                cout << "*    3.PLAY                  0.Back               *" << '\n';
                cout << "*                                                 *" << '\n';
                cout << "***************************************************" << '\n';
                cout << " Please choose your operation[0--2]:" << '\n';
                cin >> op;
                getchar();
                switch (op)
                {
                case 1:
                    // 生成数独问题及答案
                    CreateSudoku(sud, que);
                    print_sudoku(que);
                    break;
                case 2:
                    print_sudoku(sud);
                    break;
                case 3:
                {
                    cout << "当前数独问题为:" << '\n';
                    print_sudoku(que);
                    op = 1;
                    int hole = 0;
                    for (int i = 1; i < 10; i++)
                        for (int j = 1; j < 10; j++)
                            if (!que[i][j])
                                hole++;
                    while (op == 1)
                    {
                        cout << "请按‘行 列 值’的格式输入每个位置的答案" << '\n';
                        int x, y, value;
                        cin >> x >> y >> value;
                        if (sud[x][y] == value)
                        {
                            cout << "该答案正确!" << '\n';
                            if (!que[x][y])
                                hole--;
                            que[x][y] = value;
                            cout << "当前数独为:" << '\n';
                            print_sudoku(que);
                        }
                        else
                        {
                            cout << "该答案错误!" << '\n';
                        }
                        if (!hole)
                        {
                            cout << "恭喜求解成功!" << '\n';
                            break;
                        }
                        cout << "是否继续答题?(Y:1 / N:0)" << '\n';
                        cin >> op;
                    }
                    op = 3;
                    break;
                }
                case 0:
                    break;
                default:
                    cout << "请在[0--2]范围内选择操作" << '\n';
                }
                cout << "按任意键继续" << '\n';
                getchar();
            }
            op = 1;
            break;
        case 2:
            cout << "请输入待解决SAT问题cnf文件路径(用\\\\代替\\防转义):" << '\n';
            getline(cin, path);
            cout << "按任意键继续" << '\n';
            getchar();
            Sat(path);
            break;
        case 0:
            exit(0);
        default:
            cout << "请在[0--2]范围内选择操作" << '\n';
        }
        if (op == 1)
            continue;
        cout << "按任意键继续" << '\n';
        getchar();
    }
    return 0;
}

// g++ main.cpp SatSolve.cpp CnfPaster.cpp X-sudoku.cpp -o res
