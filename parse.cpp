#include "9cc.h"
#include <ctype.h>
#include <iostream>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

Token *token;     //現在着目しているトークン
char *user_input; //入力

void error_at(const char *loc, const char *fmt, ...) {
  va_list ap;        //可変長引数
  va_start(ap, fmt); //可変長引数の初期化. ap: 引数リスト,
                     // fmp:指定した引数以降を引数リストに格納

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); //*で数で表示する文字数を定義できると
  fprintf(stderr, "^ ");
  // vfprintf(resource $stream, string $format, array $values): int
  // stream: 出力先, format: 変換書式文字列, values: 出力する変数群
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1); //プログラムの終了
}

//次のトークンが期待している記号の時はトークンを読み進め、trueを返す
//それ以外はfalseを返す
bool consume(const char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op,
             token->len)) { // token->strとopをtoken->len文字だけ比較
    return false;
  }

  token = token->next;
  return true;
}

Token *consume_ident() {
  if (token->kind != TK_IDENT) {
    return nullptr;
  }

  Token *ident = token;
  token = token->next;
  return ident;
}

//次のトークンが期待している記号の時はトークンを読み進める
//それ以外はエラーを報告する
void expect(const char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    error_at(token->str, "\"%s\"ではありません", op);
  }
  token = token->next;
}

//次のトークンが数値の時はトークンを読み進める
//それ以外はエラーを報告する
int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "数ではありません");
  }

  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

bool startswitch(const char *p, const char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

//新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, const char *str, int len) {
  // malloc関数で確保して、領域を0で初期化. calloc(count, size) count: 要素数.
  // size: 要素のサイズ
  Token *tok = (Token *)calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  tok->len = len;

  return tok;
}

void tokenlize() {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  const char *p = user_input;
  while (*p) {
    if (isspace(*p)) { //引数のint型が空白か？
      p++;
      continue;
    }

    if (startswitch(p, "==") || startswitch(p, "!=") || startswitch(p, "<=") ||
        startswitch(p, ">=")) {
      // 2文字以上トークンが進むケース
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (string("+-*/()<>;=").find(*p) != std::string::npos) {
      cur = new_token(TK_RESERVED, cur, p, 1);
      p++;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      cur = new_token(TK_IDENT, cur, p, 1);
      p++;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0); //長さがわからないのでいったん0を代入
      const char *q = p;                  //現在のpのアドレスを保持
      // strtol(): 文字列を数値に変換する. 文字数に応じてpを進める
      char *p2 = nullptr;
      cur->val = strtol(p, &p2, 10);
      p = p2;
      cur->len = p - q; //アドレスを比較し、進んだ距離をlenに代入
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0); // 1つ目のTokenは空のTokenなのでnextのものを返す
  token = head.next;
}