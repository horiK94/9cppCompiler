#include "9cc.h"
#include <ctype.h>
#include <iostream>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

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
LVar *locals; //この変数を辿ることで既知かを判定

//値、変数、カッコで囲まれたステートを検知. 例) 2, a, (a + 4 == 2 * 3) != 0
// primary    = num | ident | "(" expr ")"
Node *primary() {
  if (consume("(")) {
    //次のトークンが'('なら、'('expr')'となるはず
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    Node *node = (Node *)calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar) {
      //変数がある
      node->offset = lvar->offset;
    } else {
      //未知の変数
      lvar = (LVar *)calloc(1, sizeof(LVar)); //記憶域を割り当て
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      if (locals == nullptr) {
        lvar->offset = 8;
      } else {
        lvar->offset = locals->offset + 8;
      }
      node->offset = lvar->offset;
      locals = lvar;
    }
    // offset:
    // 与えられた文字の'a'の前のローカル変数のベースポインタ(=関数呼び出し時点のRBP)からのオフセット
    return node;
  }

  //()が無ければ数字があるはず
  return new_node_num(expect_number());
}

//プラス、マイナスを検知. 例) +a, -3
// unary      = ("+" | "-")? primary
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

//積、商を検知. 例) 4 / 2, a * 3
// mul        = unary ("*" unary | "/" unary)*
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

//和や差を検知. 例) 2 + 4, a - 4
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

// 大小関係を検知. 例) 1 < a + 3, 3 >= 4, 1 <= 2 > 0
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

//等号、不等号の検知. 2 == 1 + 4 や a != 5 > 2 == 0 といったものを検知
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

//代入式か？ h = 2 や a = b = 0 == 1を検知する
// assign     = equality ("=" assign)?
Node *assign() {
  Node *node = equality();

  if (consume("=")) {
    node = new_node(ND_ASSIGN, node, assign());
  }
  return node;
}

// expr       = assign
Node *expr() { return assign(); }

//文の終端を検知する
// stmt       = expr ";" | "return" expr ";"
Node *stmt() {
  Node *node;

  if (consume("return")) {
    // Nodeの領域を1つ確保
    node = (Node *)calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr(); // Node*なのでlhsに入れる
  } else {
    node = expr();
  }

  if (!consume(";")) //;でなければfalseが返ってくる. ;ならトークンを読み進める
  {
    error_at(token->str, "';'ではないトークンです");
  }
  return node;
}

Node *code[100];

//複数行書けるように. h = 2; t = 3;
// program    = stmt*
void program() {
  int i = 0;
  while (!at_eof()) {
    code[i++] = stmt();
  }
  code[i] = nullptr;
}

//与えられたノードが変数を指しているときに、
//その変数のアドレスを計算して、それをスタックにプッシュ
void gen_lval(Node *node) {
  if (node->kind != ND_LVAR) {
    error_at((char *)node->kind, "代入の左辺値が変数ではありません");
  }

  // rbpの値（=関数呼び出し時点のRBP)をraxにコピー
  std::cout << "  mov rax, rbp\n";
  // node->offsetだけraxのアドレスを戻す
  std::cout << "  sub rax, " << node->offset << "\n";
  // rspを8バイト減らして、raxの内容(=変数の内容)を書き込む
  std::cout << "  push rax\n";
}

void gen(Node *node) {
  switch (node->kind) {
  case ND_NUM: //数字
    std::cout << "  push " << node->val << "\n";
    return;
  case ND_LVAR: //変数
    gen_lval(node);
    // gen_lval()で"push rax"をする前に戻る
    std::cout << "  pop rax\n";

    std::cout << "  mov rax, [rax]\n";
    std::cout << "  push rax\n";
    return;
  case ND_ASSIGN: //=
    // raxは変数のアドレス、rspはraxの値が書き込まれたアドレス(≠raxのアドレス)
    gen_lval(node->lhs);
    gen(node->rhs);
    // rdiには右辺の値が書き込まれる
    std::cout << "  pop rdi\n";
    // raxには元のraxの値が入る(つまり変わらない)
    std::cout << "  pop rax\n";
    std::cout << "  mov [rax], rdi\n";
    std::cout << "  push rdi\n";
    return;
  case ND_RETURN: //リターン文
    gen(node->lhs);
    //式の評価結果としてスタックに一つの値が残っているのでpopする
    //最後の式の結果はraxに入れる
    cout << "  pop rax\n";
    // rbpは関数の呼び出し時点のBPR値が代入されたアドレスを指している.
    // rspも同じアドレスを代入する
    cout << "  mov rsp, rbp\n";
    // rbpにrspがいるアドレスの中の値(=関数呼び出し時点のBRPの値)を書き込む =
    // 関数呼び出し時点の配置になる
    cout << "  pop rbp\n";
    cout << "  ret\n";
    return;
  }

  //左辺と右辺があるタイプの時
  gen(node->lhs);
  gen(node->rhs);

  std::cout << "  pop rdi\n"; //右辺(node->rhs)をrdiにpop
  std::cout << "  pop rax\n"; //左辺(node->lhs)をraxにpop

  switch (node->kind) {
  case ND_ADD:
    std::cout << "  add rax, rdi\n";
    break;
  case ND_SUB:
    std::cout << "  sub rax, rdi\n";
    break;
  case ND_MUL:
    std::cout << "  imul rax, rdi\n";
    break;
  case ND_DIV:
    std::cout
        << "  cqo\n"; // rax の値を 128bit integer に拡張し, rax, rdx(符号拡張)
    // レジスタへ格納
    std::cout << "  idiv rdi\n"; // dx, rax
    // を合わせた値を第1オペランド(左の書き方だとrdi)のレジスタの
    // 64bit 値で割る. 商を rax, 余りを rdxレジスタに格納
    break;
  case ND_EQ:
    std::cout
        << "  cmp rax, rdi\n"; // rax と rdiを比較し、フラグレジスタにセット
    std::cout
        << "  sete al\n"; // cmp命令で調べた2つのレジスタが同じならraxの下位8bit(=al)に1をセット(それ以外なら0)
    // setaは8ビットレジスタしか引数に取れないので、movzbで上位56ビットはゼロクリアした状態でraxにalをセットする
    std::cout
        << "  movzb rax, al\n"; // 上位56ビットはゼロクリアした状態でraxにalをセットする
    break;
  case ND_NE:
    std::cout << "  cmp rax, rdi\n";
    std::cout
        << "  setne al\n"; // cmp命令で調べた2つのレジスタが違うならraxの下位8bit(=al)に1をセット(それ以外なら0)
    std::cout << "  movzb rax, al\n";
    break;
  case ND_LT:
    std::cout << "  cmp rax, rdi\n";
    std::cout
        << "  setl al\n"; // cmp命令で調べた2つのレジスタが<ならraxの下位8bit(=al)に1をセット(それ以外なら0)
    std::cout << "  movzb rax, al\n";
    break;
  case ND_LE:
    std::cout << "  cmp rax, rdi\n";
    std::cout
        << "  setle al\n"; // cmp命令で調べた2つのレジスタが<=ならraxの下位8bit(=al)に1をセット(それ以外なら0)
    std::cout << "  movzb rax, al\n";
    break;
  }

  std::cout << "  push rax\n"; //結果(=rax)をpushする
}