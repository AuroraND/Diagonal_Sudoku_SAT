#ifndef SUD_H_INCLUDED
#define SUD_H_INCLUDED

// 函数
void CreateSudokuRule();

int isvalid(int sud[][10], int row, int col, int value);

void CreateSudoku(int sud[][10], int que[][10]);

void DigHole(Answer *ans, int sud[][10], VarTable var_watch[], int branchdecision[]);

void print_sudoku(int sud[][10]);

#endif