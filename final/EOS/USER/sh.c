/************** sh.c file ***************/
// See page 228 of Embedded and real-time operating system (WANG 2017)

#include "ucode.c"
#define S_ISDIR(m) ((m >> 9) == 040)
#define S_ISREG(m) ((m >> 12) == 010)
#define S_ISLNK(m) ((m >> 12) == 012)

char debug[128];

int display_commands();


int main(int argc, char *argv[])
{
  int i;
  char cwd[128];
  char uline[256];
  char cmd[32];
  char args[224];
  printf("this is sh\n");

  for (i = 0; i < argc; i++)
  {
    printf("argv[%d] = %s\n", i, argv[i]);
  }

  int pid, status;
  while (1)
  {
    bzero(cwd, 128);
    bzero(uline, 256);
    bzero(cmd, 32);
    bzero(args, 224);
    display_commands();
    getcwd(cwd);
    printf("\n%s:", cwd);
    gets(uline);

    /*
    int j = 0;
    for (i = 0; i < strlen(uline) && uline[i] != ' '; i++)
    {
      cmd[j] = uline[i];
      j++;
    }
    cmd[j] = '\0';
    */

    int arg_index = 0;
    for (i = 0; i < strlen(uline); i++)
    {
      if (uline[i] == ' ')
      {
        arg_index++;
        ;
      }
      int k = 0;
      if (arg_index == 0)
      {
        while (uline[i] != ' ' && i < strlen(uline) && k < 32)
        {
          cmd[k] = uline[i];
          i++;
          k++;
        }
        arg_index++;
        cmd[k] = '\0';
      }
      else
      {
        while (i < strlen(uline) && k < 224)
        {
          args[k] = uline[i];
          i++;
          k++;
        }
        args[k] = '\0';
      }
    }

    printf("\ncmd=%s\nargs=%s\n", cmd, args);

    // EXIT
    if (!strcmp(cmd, "exit"))
    {
      exit(0);
    }

    // LOGOUT
    if (!strcmp(cmd, "logout"))
    {
      exit(0);
    }

    // CD
    if (!strcmp(cmd, "cd"))
    {
      if (strlen(args) == 0)
      {
        printf("cd - enter a dir name.\n");
        continue;
      }
      if (!strcmp(cwd, "/") && !strcmp(args, ".."))
      {
        printf("cd - you can't cd there.\n");
        continue;
      }

      chdir(args);
      continue;
    }

    pid = fork();
    if (pid)
    {
      pid = wait(&status);
      continue;
    }
    else
    {
      do_pipe(uline, 0);
    }

    /*
    // fork a child to exec uline
    pid = fork();
    if (pid)
    {
      pid = wait(&status);
    }
    else
    {
      printf("uline=%s\n", uline);
      // gets(debug);
      exec(uline);
    }
    */
  }

  // shouldn't ever get here
  printf("end of sh\n");

  exit(0);
}

int do_pipe(char *uline, int *pd)
{
  if (pd) // has a pipe passed in, as WRITER on pipe pd;
  {
    close(pd[0]);
    dup2(pd[1], 1);
    close(pd[1]);
  }
  // divide cmdline into head, tail by rightmost pipe symbol
  char head[128], tail[128];
  bzero(head, 128); bzero(tail, 128);

  if (get_head_tail(uline, head, tail) > 0) // if hasPipe == true
  {
    printf("head=%s\ntail=%s\n", head, tail);
    int lpd[2];
    pipe(lpd);

    pid = fork();
    if (pid)  // PARENT as READER on lpd
    {
      close(lpd[1]);
      dup2(lpd[0], 0);
      //close(lpd[0]);
      do_command(tail);
    }
    else
    {
      do_pipe(head, lpd);
    }
  }
  else
  {
    do_command(uline);
  }
}

int do_command(char *cmdline)
{
  char head[128];
  char fname[64];
  bzero(head, 128); bzero(fname, 64);
  int e = 0;
  for (int i = 0; i < strlen(cmdline); i++)
  {
    if (cmdline[i] == '>' || cmdline[i] == '<')
    {
      int j = 0; int k = i;
      // copy redirect target to fname
      while (cmdline[k] != ' ')
      {
        k++;
      }
      while (j < strlen(cmdline) && k < strlen(cmdline))
      {
        if (cmdline[k] == '>')
        {
          k++;
          continue;
        }
        if (cmdline[k] == ' ' && strlen(fname) == 0)
        {
          k++;
          continue;
        }
        if (cmdline[k] == ' ')
        {
          fname[j] == '\0';
          break;
        }
        else
        {
          fname[j] = cmdline[k];
          j++; k++;
        }
      }
      printf("io redirect fname=%s\n", fname);

      if (cmdline[i+1] == '>')
      {
        int nfd = open(fname, O_WRONLY | O_APPEND | O_CREAT);
        dup2(nfd, 1);
        i++;
      }
      else if (cmdline[i] == '>')
      {
        int nfd = open(fname, O_WRONLY | O_CREAT);
        dup2(nfd, 1);
      }
      else if (cmdline[i] == '<')
      {
        int nfd = open(fname, O_WRONLY | O_CREAT);
        dup2(nfd, 0);
      }
      break;
    }
    else
    {
      head[e] = cmdline[i];
      e++;
    }
  }
  printf("\ndo_command head=%s\n", head);
  head[e] = '\0';
  exec(head);
}

int get_head_tail(char *cmdline, char *head, char *tail)
{
  int flag = 0;
  for (int i = 0; i < strlen(cmdline); i++)
  {
    if (cmdline[i] == '|')
    {
      flag = i;
    }
  }
  if (flag == 0)
  {
    strcpy(head, cmdline);
    return 0;
  }

  int j = 0;
  for (int i = 0; i < flag - 1; i++)
  {
    head[j] = cmdline[i];
    j++;
  }
  head[j] == '\0';
  j = 0;
  for (int i = flag + 1; i < strlen(cmdline); i++)
  {
    tail[j] = cmdline[i];
    j++;
  }
  tail[j] = '\0';
  j = 0;

  return 1;
}

int display_commands()
{
  STAT mystat;
  STAT *mystat_ptr = &mystat;
  DIR *dp;
  char buf[1024], fname[128], *cp;
  int bin_fd = open("/bin", O_RDONLY);
  int r = 0;
  printf("\nCOMAND List:\n");
  while ((r = read(bin_fd, buf, 1024)) > 0)
  {
    // printf("read %d bytes\n", r);
    cp = buf;
    dp = (DIR *)buf;

    while (cp < 1024 + buf)
    {
      strncpy(fname, dp->name, dp->name_len);
      // printf("fname=%s\n", fname);
      int s = stat(fname, mystat_ptr);
      // printf("st->mode=%d\n", mystat.st_mode);
      // gets(debug);

      // REG FILE, don't list . or ..
      if (mystat.st_mode == 16877 && fname[0] != '.')
      {
        printf("%s, ", fname);
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  printf("\n");
}