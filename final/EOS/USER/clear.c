/************** clear.c file ***************/

#include "ucode.c"

int main(int argc, char *argv[])
{
  int i;
  
  printf("this is a Clear\n");

  for (i=0; i<argc; i++){
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  printf("\e[1;1H\e[2J");
  printf("cleared\n");

  exit(0);
}
