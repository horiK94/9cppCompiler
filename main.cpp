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
  int len; //トークンの長さ(比較演算子は2文字以上もあり得るため)
};

Token *token;     //現在着目しているトークン
char *user_input; //入力

void error_at(char *loc, char *fmt, ...) {
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
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op,
             token->len)) { // token->strとopをtoken->len文字だけ比較
    return false;
  }

  token = token->next;
  return true;
}

//次のトークンが期待している記号の時はトークンを読み進める
//それ以外はエラーを報告する
void expect(char *op) {
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

bool startswitch(char *p, char *q) { return memcmp(p, q, strlen(q)) == 0; }

//新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  // malloc関数で確保して、領域を0で初期化. calloc(count, size) count: 要素数.
  // size: 要素のサイズ
  Token *tok = (Token *)calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  tok->len = len;

  return tok;
}

Token *tokenlize(char *input) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  char *p = input;
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

    if (string("+-*/()<>").find(*p) != std::string::npos) {
      cur = new_token(TK_RESERVED, cur, p, 1);
      p++;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0); //長さがわからないのでいったん0を代入
      char *q = p;                        //現在のpのアドレスを保持
      // strtol(): 文字列を数値に変換する. 文字数に応じてpを進める
      cur->val = strtol(p, &p, 10);
      cur->len = p - q; //アドレスを比較し、進んだ距離をlenに代入
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0); // 1つ目のTokenは空のTokenなのでnextのものを返す
  return head.next;
}

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // 整数
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
} NodeKind;

typedef struct Node node;

struct Node {
  NodeKind kind; //ノードの型
  Node *lhs;     //左辺
  Node *rhs;     //右辺
  int val;       // kindがND_NUMの時使用
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = (Node *)calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = (Node *)calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *expr();

// primary = "(" expr ")" | num
Node *primary() {
  if (consume("(")) {
    //次のトークンが'('なら、'('expr')'となるはず
    Node *node = expr();
    expect(")");
    return node;
  }

  //()が無ければ数字があるはず
  return new_node_num(expect_number());
}

// unary = ("+" | "-")? primary
Node *unary() {
  if (consume("+")) {
    return primary();
  } else if (consume("-")) {
    // primaryに対して、0-primaryに置き換える
    return new_node(ND_SUB, new_node_num(0), primary());
  } else {
    return primary();
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_node(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

// add        = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+")) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume("-")) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<=")) {
      node = new_node(ND_LE, node, add());
    } else if (consume("<")) {
      node = new_node(ND_LT, node, add());
    } else if (consume(">=")) {
      node = new_node(ND_LE, add(), node);
    } else if (consume(">")) {
      node = new_node(ND_LT, add(), node);
    } else {
      return node;
    }
  }
}

// equality   = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("==")) {
      node = new_node(ND_EQ, node, relational());
    } else if (consume("!=")) {
      node = new_node(ND_NE, node, relational());
    } else {
      return node;
    }
  }
}

// expr       = equality
Node *expr() { return equality(); }

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    cout << "  push " << node->val << '\n';
    return;
  }

  //左辺と右辺があるタイプの時
  gen(node->lhs);
  gen(node->rhs);

  cout << "  pop rdi\n"; //右辺(node->rhs)をrdiにpop
  cout << "  pop rax\n"; //左辺(node->lhs)をraxにpop

  switch (node->kind) {
  case ND_ADD:
    cout << "  add rax, rdi\n";
    break;
  case ND_SUB:
    cout << "  sub rax, rdi\n";
    break;
  case ND_MUL:
    cout << "  imul rax, rdi\n";
    break;
  case ND_DIV:
    cout << "  cqo\n"; // rax の値を 128bit integer に拡張し, rax, rdx(符号拡張)
                       // レジスタへ格納
    cout
        << "  idiv rdi\n"; // dx, rax
                           // を合わせた値を第1オペランド(左の書き方だとrdi)のレジスタの
                           // 64bit 値で割る. 商を rax, 余りを rdxレジスタに格納
    break;
  case ND_EQ:
    cout << "  cmp rax, rdi\n"; // rax と rdiを比較し、フラグレジスタにセット
    cout
        << "  sete al\n"; // cmp命令で調べた2つのレジスタが同じならraxの下位8bit(=al)に1をセット(それ以外なら0)
    // setaは8ビットレジスタしか引数に取れないので、movzbで上位56ビットはゼロクリアした状態でraxにalをセットする
    cout
        << "  movzb rax, al\n"; // 上位56ビットはゼロクリアした状態でraxにalをセットする
    break;
  case ND_NE:
    cout << "  cmp rax, rdi\n";
    cout
        << "  setne al\n"; // cmp命令で調べた2つのレジスタが違うならraxの下位8bit(=al)に1をセット(それ以外なら0)
    cout << "  movzb rax, al\n";
    break;
  case ND_LT:
    cout << "  cmp rax, rdi\n";
    cout
        << "  setl al\n"; // cmp命令で調べた2つのレジスタが<ならraxの下位8bit(=al)に1をセット(それ以外なら0)
    cout << "  movzb rax, al\n";
    break;
  case ND_LE:
    cout << "  cmp rax, rdi\n";
    cout
        << "  setle al\n"; // cmp命令で調べた2つのレジスタが<=ならraxの下位8bit(=al)に1をセット(それ以外なら0)
    cout << "  movzb rax, al\n";
    break;
  }

  cout << "  push rax\n"; //結果(=rax)をpushする
}

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
  cout << "  ret\n";
  return 0;
}