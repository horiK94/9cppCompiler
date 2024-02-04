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

  char *p = argv[1];

  cout << ".intel_syntax noprefix\n";
  cout << ".globl main\n";
  cout << "main:\n";
  cout << "  mov rax, " << strtoll(p, &p, 10) << '\n';

  while (*p) {
    if (*p == '+') {
      p++;
      cout << "  add rax, " << strtoll(p, &p, 10) << "\n";
      continue;
    }
    if (*p == '-') {
      p++;
      cout << "  sub rax, " << strtoll(p, &p, 10) << "\n";
      continue;
    }

    cout << "予期しない文字: " << *p;
    return 1;
  }

  cout << "  ret\n";
  return 0;
}