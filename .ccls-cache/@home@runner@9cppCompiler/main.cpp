#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

//コマンドの第一引数に直接コードを渡す仕組み
// argc: 引数の個数
// argv; 引数の配列 ex. $ ./a.out 100 abc →　"./a.out","100","abc"
int main(int argc, char **argv) {
  if (argc != 2) {
    cout << "引数の個数が正しくありません。" << endl;
    return 1;
  }

  cout << ".intel_syntax noprefix\n";
  cout << ".globl main\n";
  cout << "main:\n";
  cout << "  mov rax, " << atoi(argv[1]) << '\n';
  cout << "  ret\n";
  return 0;
}