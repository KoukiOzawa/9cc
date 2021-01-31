#include"9cc.h"

char *read_file(char *fname){
  FILE *fp;
  user_input = malloc(1000);
  memset(user_input, 0, 1000);

  fp = fopen(fname, "r");
  if(fp == NULL){
    error("cant open file\n");
  }
  size_t offset=0;
  int size = fread(user_input, 1, 1000, fp);

  //ファイルの中身が０　もしくは最後が改行じゃない
  if(size == 0 || user_input[size-1] != '\n'){
    user_input[size++] = '\n';
  }
  fclose(fp);
  user_input[size] = '\0';
  return user_input;
}

int main(int argc, char **argv){
  if(argc != 2){
    fprintf(stderr, "引数の個数が正しくありません");
    return 1;
  }
  
  user_input = read_file(argv[1]);
  token = tokenize();
  locals = calloc(1,sizeof(LVar));
  locals->offset = 0;
  program();

  //アセンブリの前半部分
  printf(".intel_syntax noprefix\n");//おまじない
  int i=0;
  for(;;){
    Function *fc = code[i++];
    if(fc == NULL)
      break;
    printf(".globl %s\n",fc->name);//main ラベルを外部に公開
    printf("%s:\n",fc->name);//main fun start

    //プロローグ
    //変数26個分の領域を確保する
    printf(" push rbp\n");
    printf(" mov rbp, rsp\n");
    printf(" sub rsp, 208\n");

    //
    for(Node *node = fc->body; node; node=node->next){
      gen(node);
    }

    printf(" pop rax\n");

    //エピローグ
    printf(" mov rsp, rbp\n");
    printf(" pop rbp\n");
    printf(" ret\n");
  }
  return 0;
}
