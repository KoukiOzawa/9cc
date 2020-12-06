#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

typedef enum{
  TK_RESERVED, //記号
  TK_NUM, //整数トークン
  TK_EOF, //入力の終わりを表すトークン
}TokenKind

typedef struct Token Token;

struct Token{
  TokenKind kind; //トークンの種類
  Token *next;
  int val; //数値
  char *str; //トークン文字列
};

Token *token;

void error(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
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

void expect(char op){
  if(token->kind != TK_RESERVED || token->str[0] != op)
      error("'%c'ではありません", op);
  token = token->next;
}

int expect_number(){
  if(token->kind != TK_NUM)
    error("数ではありません")
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

Token *tokenize(cahr *p){
  Token head;
  head.next = NULL;
  Token *cur = &head;

  //pには文字列、文字列の最後にはヌル文字が入っている
  while(*p){
    if(isspace(*p)){
      p++;
      continue;
    }

    if(*p == '+' || *p == '-'){
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if(isdigit(*p)){
      cur = new_token(TK_NUM, cur, p++);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error("トークナイズできません");
  }
  new_token(TK_EOF, cur, p);
  //最初のtokenはメモリ領域を確保していない
  //つまり中身空の目印みたいなのも
  return head.next;
}

int main(int argc, char **argv){
  if(argc != 2){
    fprintf(stderr, "引数の個数が正しくありません");
    return 1;
  }

  token = tokenize(argv[1]);
  
  char *p = argv[1];
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  //式の最初は数である必要がある
  printf(" mov rax, %ld\n", strtol(p, &p, 10));

  while(!at_eof()){
    if(consume('+')){
      printf(" add rax, %ld", expect_number());
      continue;
    }

    expect('-');
    printf(" sub rax, %ld", expect_number());
  }

  printf(" ret\n");
  return 0;
}
