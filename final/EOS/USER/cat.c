/************** test.c file ***************/

#include "ucode.c"

int main(int argc, char *argv[ ])
{
  int i, r;
  char buf[1024];
  
  //printf("this is cat\n");
  /*
  for (i=0; i<argc; i++){
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  */

  // must supply at least 1 additional argument to cat
  if (argv == 1)
  {
    printf("cat - error: must enter some arguments.\n");
    return 0;
  }

  // close stdin
  close(0);
  // open specified file for READ
  open(argv[1], O_RDONLY);

  while ((r = read(0, buf, 1024)) > 0)
  {
    buf[1023] = '\0';
    printf("%s", buf);
  }

  close(argv[1]);
  // printf("\nend of cat\n");

  exit(0);
}
