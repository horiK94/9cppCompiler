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

typedef struct Node node;

struct Node {
  NodeKind kind; //ノードの型
  Node *lhs;     //左辺
  Node *rhs;     //右辺
  int val;       // kindがND_NUMの時使用
};

extern Token *token;     //現在着目しているトークン
extern char *user_input; //入力

bool consume(char *op);
Token *tokenlize(char *input);
Node *expr();
void gen(Node *node);