/************** grep.c file ***************/

#include "ucode.c"

int main(int argc, char *argv[])
{
  int i, fd;
  char pattern[32];

  // printf("this is grep\n");
  /*
  for (i=0; i<argc; i++){
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  */
  if (argc == 1)
  {
    printf("\ngrep - error: enter some args.\n");
    return -1;
  }
  else if (argc == 2) // embedded operation, we shall read from stdin
  {
    fd = 0;
  }
  else if (argc > 2)
  {
    fd = open(argv[2], O_RDONLY); // argv[2] is fname, argv[1] is pattern
  }
  if (fd < 0)
  {
    printf("grep - error: bad file name.\n");
    return -1;
  }

  // grab the pattern, if pattern > 32, grab first 32 chars
  strlen(argv[1]) < 32?strncpy(pattern, argv[1], strlen(argv[1])):strncpy(pattern, argv[1], 32);

  

  // printf("end of grep\n");
}
