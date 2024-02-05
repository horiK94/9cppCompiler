#include <ctype.h>
#include <iostream>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

typedef enum {
  TK_RESERVED, //記号
  TK_NUM,      //整数トークン
  TK_EOF,      //入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind; //トークンの型
  Token *next;    //次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // kindがTK_RESERVEDの場合、その文字列
};

Token *token; //現在着目しているトークン

void error(char *fmt, ...) {
  va_list ap;        //可変長引数
  va_start(ap, fmt); //可変長引数の初期化. ap: 引数リスト,
                     // fmp:指定した引数以降を引数リストに格納
  // vfprintf(resource $stream, string $format, array $values): int
  // stream: 出力先, format: 変換書式文字列, values: 出力する変数群
  vfprintf(stderr, fmt, ap);
  cout << '\n';
  exit(1); //プログラムの終了
}

//次のトークンが期待している記号の時はトークンを読み進め、trueを返す
//それ以外はfalseを返す
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    return false;
  }
  token = token->next;
  return true;
}

//次のトークンが期待している記号の時はトークンを読み進める
//それ以外はエラーを報告する
void except(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    error("'%c'ではありません", op);
  }
  token = token->next;
}

//次のトークンが数値の時はトークンを読み進める
//それ以外はエラーを報告する
int except_number() {
  if (token->kind != TK_NUM) {
    error("数ではありません");
  }

  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

//新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
  // malloc関数で確保して、領域を0で初期化. calloc(count, size) count: 要素数.
  // size: 要素のサイズ
  Token *tok = (Token *)calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;

  return tok;
}

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