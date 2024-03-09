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

  //トークナイズしてパース
  //結果はcodeに保存
  user_input = argv[1];
  tokenlize();
  program();

  // アセンブリの前半部分を出力
  cout << ".intel_syntax noprefix\n";
  cout << ".globl main\n";
  cout << "main:\n";

  //プロローグ
  //変数26個分の領域を確保
  cout << "  push rbp\n";     // rbpのアドレスをpush
  cout << "  mov rbp, rsp\n"; // rspに格納されている値をrbpにコピー
  cout << "  sub rsp, 208\n"; // 26個分の領域確保

  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // 式の評価結果としてスタックに一つの値が残っている
    // はずなので、スタックが溢れないようにポップしておく
    cout << "  pop rax\n";
  }

  //最後の式の結果がraxに残っているので返り値にする
  cout << "  mov rsp, rbp\n";
  // rbpに結果がpushされているので、rspにロード(=pop)する
  cout << "  pop rbp\n";
  cout << "  ret\n ";
  return 0;
}