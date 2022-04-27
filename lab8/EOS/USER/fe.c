/************** test.c file ***************/

#include "ucode.c"

int main(int argc, char *argv[ ])
{
  int i;
  char cmdline[128];
  bzero(cmdline, 128);
  printf("**this is fe.c**\n");
  int j = 0;
  for (i=0; i<argc; i++)
  {
    printf("argv[%d] = %s\n", i, argv[i]);
    if (i == 0)
    {
      continue;
    }
    
    strcat(cmdline, argv[i]);
    if (i+1 != argv)
    {
      strcat(cmdline, " ");
    }
  }
  printf("********\n");
  printf("cmdline=%s\n", cmdline);

  int status;
  int pid = fe(cmdline);
  pid = wait(&status);
  printf("PID=%d, STATUS = %d\n", pid, status);

  printf("**end of fe.c**\n");

  exit(0);
}
