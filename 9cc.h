typedef enum {
  TK_RESERVED, //記号
  TK_IDENT,    //識別子
  TK_NUM,      //整数トークン
  TK_EOF,      //入力の終わりを表すトークン
  TK_RETURN,   // returnトークン
} TokenKind;

struct Token {
  TokenKind kind;  //トークンの型
  Token *next;     //次の入力トークン
  int val;         // kindがTK_NUMの場合、その数値
  const char *str; // kindがTK_RESERVEDの場合、その文字列
  int len; //トークンの長さ(比較演算子は2文字以上もあり得るため)
};

typedef enum {
  ND_ADD,    // +
  ND_SUB,    // -
  ND_MUL,    // *
  ND_DIV,    // /
  ND_NUM,    // 整数
  ND_EQ,     // ==
  ND_NE,     // !=
  ND_LT,     // <
  ND_LE,     // <=
  ND_ASSIGN, //=
  ND_LVAR,   //ローカル変数
  ND_RETURN, // return文
} NodeKind;

struct Node {
  NodeKind kind; //ノードの型
  Node *lhs;     //左辺
  Node *rhs;     //右辺
  int val;       // kindがND_NUMの時使用
  int offset;    // kindがND_LVARの時使用
};

//ローカル変数の型
struct LVar {
  LVar *next;       //次の変数かnullptr
  const char *name; //変数名
  int len;          //長さ
  int offset;       // RBPからのオフセット
};

extern LVar *locals; //この変数を辿ることで既知かを判定

bool consume(const char *op);
Token *consume_ident();
int expect_number();
void expect(const char *op);
void tokenlize();
void program();
void gen(Node *node);
bool at_eof();
void error_at(const char *loc, const char *fmt, ...);
LVar *find_lvar(Token *tok);

extern Token *token;     //現在着目しているトークン
extern char *user_input; //入力
extern Node *code[100];
