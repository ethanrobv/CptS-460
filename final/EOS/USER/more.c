/************** more.c file ***************/

#include "ucode.c"

int more_helper(int num_lines, char *buf);

int main(int argc, char *argv[])
{
  int i, fd;
  STAT mystat, *mystat_ptr = &mystat;
  char tty[16];

  //printf("this is more\n");
  /*
  for (i = 0; i < argc; i++)
  {
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  */

  //Look at lines 1242 - 1244 on
  //https://github.com/util-linux/util-linux/blob/master/text-utils/more.c
  //essentially, close(stdin), then open(tty). But first, we need to keep
  //the OG file descriptor.
  //**we have to give the proc kbd interrupts while this program executes**
  if (argc == 1) // open communication with teletypewriter (for piping, I/O red)
  {
    fd = dup(0);
    close(0);
    bzero(tty, 16);
    gettty(tty);
    open(tty, O_RDONLY);
  }
  else
  {
    fd = open(argv[1], O_RDONLY);
  }

  if (fd < 0)
  {
    printf("more - error: file doesn't exist.\n");
    return -1;
  }

  stat(argv[1], mystat_ptr);

  int fsize = mystat.st_size;
  int r;
  int total_bytes_read = 0;
  char buf[1024];
  bzero(buf, 1024);
  int num_lines = 0;
  while (r = read(fd, buf, 1024))
  {
    for (i = 0; i < r; i++)
    {
      num_lines = 0;
      char c = getc();
      if (c == ' ') // if spacebar, get many lines
      {
        while (num_lines < 80 && i < r)
        {
          if (buf[i] == '\r')
          {
            num_lines++;
          }
          putc(buf[i]);
          i++;
        }
      }
      else if (c == '\n' || c == '\r') // if enter, get one line
      {
        while (num_lines < 1 && i < r)
        {
          if (buf[i] == '\r')
          {
            num_lines++;
          }
          putc(buf[i]);
          i++;
        }
      }
      else if (c == 'q') // terminate
      {
        return -2;
      }
    }
    bzero(buf, 1024);
  }
  //printf("end of more\n");
}
