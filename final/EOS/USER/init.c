/************** init.c file ***************/

#include "ucode.c"

int parent();

int console;
int console2, console3;

int main(int argc, char *argv[])
{
  int in2 = open("/dev/ttyS0", O_RDONLY);
  int out2 = open("/dev/ttyS0", O_WRONLY);

  printf("INIT main: fork a login proc on ttyS0\n");
  console2 = fe("login /dev/ttyS0");
  close(in2);
  close(out2);

  int in3 = open("/dev/ttys1", O_RDONLY);
  int out3 = open("/dev/ttyS1", O_WRONLY);

  printf("INIT main: fork a login proc on ttyS1\n");
  console3 = fe("login /dev/ttyS1");
  close(in3);
  close(out3);

  // FDs for terminal I/O
  int in = open("/dev/tty0", O_RDONLY);  // file descriptor 0
  int out = open("/dev/tty0", O_WRONLY); // for display to console

  printf("INIT main: fork a login proc on tty0\n");
  console = fe("login /dev/tty0");
  close(in);
  close(out);

  while (1)
  {
    int status;

    /*
    int in2 = open("/dev/ttyS0", O_RDONLY);
    int out2 = open("/dev/ttyS0", O_WRONLY);

    printf("INIT main: fork a login proc on ttyS0\n");
    console2 = fe("login /dev/ttyS0");
    close(in2);
    close(out2);

    int in3 = open("/dev/ttys1", O_RDONLY);
    int out3 = open("/dev/ttyS1", O_WRONLY);

    printf("INIT main: fork a login proc on ttyS1\n");
    console3 = fe("login /dev/ttyS1");
    close(in3);
    close(out3);

    // FDs for terminal I/O
    int in = open("/dev/tty0", O_RDONLY);  // file descriptor 0
    int out = open("/dev/tty0", O_WRONLY); // for display to console

    printf("INIT main: fork a login proc on tty0\n");
    console = fe("login /dev/tty0");
    close(in);
    close(out);
    */

    int pid = wait(&status);
    if (pid == console)
    {
      int in = open("/dev/tty0", O_RDONLY);  // file descriptor 0
      int out = open("/dev/tty0", O_WRONLY); // for display to console
      console = fe("login /dev/tty0");
      close(in);
      close(out);
    }
    else if (pid == console2)
    {
      int in2 = open("/dev/ttyS0", O_RDONLY);
      int out2 = open("/dev/ttyS0", O_WRONLY);
      console2 = fe("login /dev/ttyS0");
      close(in2);
      close(out2);
    }
    else if (pid == console3)
    {
      int in3 = open("/dev/ttys1", O_RDONLY);
      int out3 = open("/dev/ttyS1", O_WRONLY);
      console3 = fe("login /dev/ttyS1");
      close(in3);
      close(out3);
    }

    printf("INIT ttyS0: I just buried an orphan child proc %d\n", pid);
  }
  /*
  console = fork();

  if (console)  // parent
  {
    parent();
  }

  int in2, out2;
  in2 = open("/dev/ttyS0", O_RDONLY);
  out2 = open("/dev/ttys0", O_WRONLY);

  console2 = fork();
  if (console2)
  {
    user2();
  }
  else  // child: exec to login on tty0
  {
    exec("login /dev/tty0");
  }
  */
}

int user2()
{
  int pid, status;
  while (1)
  {
    printf("INIT ttyS0: wait for ZOMBIE child\n");
    pid = wait(&status);
    if (pid == console2) // if console login process died
    {
      printf("INIT: forks a new console\n");
      console2 = fork(); // fork another

      if (console2)
      {
        continue;
      }
      else
      {
        exec("login /dev/ttyS0"); // new console login process
      }
    }
    printf("INIT ttyS0: I just buried an orphan child proc %d\n", pid);
  }
}

int parent() // P1's code
{
  int pid, status;
  while (1)
  {
    printf("INIT tty0: wait for ZOMBIE child\n");
    pid = wait(&status);
    if (pid == console) // if console login process died
    {
      printf("INIT: forks a new console\n");
      console = fork(); // fork another

      if (console)
      {
        continue;
      }
      else
      {
        exec("login /dev/tty0"); // new console login process
      }
    }
    printf("INIT tty0: I just buried an orphan child proc %d\n", pid);
  }
}