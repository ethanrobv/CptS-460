/************** cat.c file ***************/

#include "ucode.c"

char debug[12];

int main(int argc, char *argv[])
{
  char buf[1024];
  int nbytes = 1024;
  char tty[64];
  char c;
  int fd;
  int bytes_left;
  char n = '\n';
  char r = '\r';

  if (argc == 1)
  {
    fd = 0;
    gettty(tty);
    open(tty, 0);
    while (read(fd, &c, 1) > 0)
    {
      write(1, &c, 1);

      if (c == '\n')
      {
        write(2, &r, 1);
      }
      else if (c == '\r')
      {
        write(1, &n, 1);
        write(2, &r, 1);
      }
    }
  }
  else
  {

    fd = open(argv[1], 0);

    if (fd < 0)
    {
      printf("cat - error: bad fname arg.\n");
      exit(-1);
    }

    STAT mystat, *mystat_ptr = &mystat;

    stat(argv[1], mystat_ptr);

    bytes_left = mystat.st_size;

    while (bytes_left > 0)
    {
      if (bytes_left < 1024)
      {
        nbytes = bytes_left;
      }

      read(fd, buf, nbytes);

      for (int i = 0; i < nbytes; i++)
      {
        c = buf[i];

        write(1, &c, 1);

        if (c == '\n')
        {
          write(2, &r, 1);
        }
        else if (c == '\r')
        {
          write(1, &n, 1);
          write(2, &r, 1);
        }
      }
      bytes_left -= nbytes;
    }

    close(fd);

    return 1;
  }
}