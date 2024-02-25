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