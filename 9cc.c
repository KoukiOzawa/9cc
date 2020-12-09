#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<stdarg.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

typedef enum{
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,
} NodeKind;

typedef struct Node Node;

//抽象構文木のノードの型
struct Node{
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;
};

Node *expr();
Node *primary();
Node *mul();
Node *unary();
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

typedef enum{
  TK_RESERVED, //記号
  TK_NUM, //整数トークン
  TK_EOF, //入力の終わりを表すトークン
}TokenKind;

typedef struct Token Token;

struct Token{
  TokenKind kind; //トークンの種類
  Token *next;
  int val; //数値
  char *str; //トークン文字列
};

Token *token;
char *user_input;

void error_at(char *loc, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  //* pos個分表示するという意味
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

//期待している記号の時には、トークンを1つ読み進めて
//真を返す。
bool consume(char op){
  if(token->kind != TK_RESERVED || token->str[0] != op)
      return false;
  token = token->next;
      return true;
}

//トークンを一つ進める
void expect(char op){
  if(token->kind != TK_RESERVED || token->str[0] != op)
      error_at(token->str, "'%c'ではありません", op);
  token = token->next;
}

//トークンを一つ進めて、数字を返す
int expect_number(){
  if(token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof(){
  return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str){
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

Token *tokenize(){
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  //pには文字列、文字列の最後にはヌル文字が入っている
  while(*p){
    if(isspace(*p)){
      p++;
      continue;
    }

    if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')'){
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if(isdigit(*p)){
      cur = new_token(TK_NUM, cur, p);
      //strtolでポインタを進めている
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "トークナイズできません");
  }
  new_token(TK_EOF, cur, p);
  //最初のtokenはメモリ領域を確保していない
  //つまり中身空の目印みたいなのも
  return head.next;
}


//2項演算子
Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

//数値用
Node *new_node_num(int val){
  Node *node = calloc(1,sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

//expr = mul ("+" mul | "-" mul)*
Node *expr(){
  Node *node = mul();

  for(;;){
    if(consume('+'))
      node = new_node(ND_ADD, node, mul());
    else if(consume('-'))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

//単項演算子を追加unary
//mul = unary("*" unary | "/" unary)*
Node *mul(){
  Node *node = unary();

  for(;;){
    if(consume('*'))
      node = new_node(ND_MUL, node, unary());
    else if(consume('/'))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

//unary = ("+"  | "/" )? primary
Node *unary(){
  if(consume('+'))
    return primary();
  if(consume('-'))
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}

//primary = num | "(" expr ")"
Node *primary(){
  if(consume('(')){
    Node *node = expr();
    expect(')');
    return node;
  }

  //今の所一番最初のexpr()でここに来る
  return new_node_num(expect_number());
}

void gen(Node *node){
  if(node->kind == ND_NUM){
    printf(" push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf(" pop rdi\n");
  printf(" pop rax\n");

  switch(node->kind){
  case ND_ADD:
    printf(" add rax, rdi\n");
    break;
  case ND_SUB:
    printf(" sub rax, rdi\n");
    break;
  case ND_MUL:
    printf(" imul rax, rdi\n");
    break;
  case ND_DIV:
    printf(" cqo\n");
    printf(" idiv rdi\n");
    break;
  }

  printf(" push rax\n");
}

int main(int argc, char **argv){
  if(argc != 2){
    fprintf(stderr, "引数の個数が正しくありません");
    return 1;
  }
  
  user_input = argv[1];
  token = tokenize();
  Node *node = expr();
  
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  gen(node);
  printf(" pop rax\n");
  printf(" ret\n");
  return 0;
}
