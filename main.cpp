#include "9cc.h"
#include <ctype.h>
#include <iostream>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

//コマンドの第一引数に直接コードを渡す仕組み
// argc: 引数の個数
// argv; 引数の配列 ex. $ ./a.out 100 abc →　"./a.out","100","abc"
int main(int argc, char **argv) {
  if (argc != 2) {
    cout << "引数の個数が正しくありません。" << endl;
    return 1;
  }

  user_input = argv[1];
  token = tokenlize(user_input);
  Node *node = expr();

  cout << ".intel_syntax noprefix\n";
  cout << ".globl main\n";
  cout << "main:\n";

  //抽象構文木を下りながらコード生成
  gen(node);

  //スタックトップに結果がpushされているので、raxにロード(=pop)する
  cout << "  pop rax\n";
  cout << "  ret\n ";
  return 0;
}