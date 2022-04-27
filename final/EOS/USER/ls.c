/************** test.c file ***************/

#include "ucode.c"

#define S_ISDIR(m) ((m >> 9) == 040)
#define S_ISREG(m) ((m >> 12) == 010)
#define S_ISLNK(m) ((m >> 12) == 012)

int ls_file(char *fname);
int ls_dir(char *dname);

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int main(int argc, char *argv[ ])
{
  int i;
  char path[128], xname[128];
  bzero(path, 128); bzero(xname, 128);
  
  printf("this is a ls\n");

  for (i=0; i<argc; i++){
    printf("argv[%d] = %s\n", i, argv[i]);
  }

  if (argc == 1)
  {
    getcwd(path);
    ls_dir(path);
  }
  else
  {
    STAT mystat, *mystat_ptr = &mystat;
    strcpy(xname, argv[1]);
    if (xname[0] == '-')
    {
      // do flags
    }
    else
    {
      stat(xname, mystat_ptr);
      if (S_ISREG(mystat.st_mode))
      {
        ls_file(xname);
      }
      else
      {
        ls_dir(xname);
      }
    }
  }

  printf("end of ls\n");

  exit(0);
}

int ls_file(char *fname)
{
  STAT mystat;
  STAT *mystat_ptr = &mystat;

  stat(fname, mystat_ptr);
  if (S_ISREG(mystat.st_mode))
  {
    putc('-');
  }
  else if (S_ISDIR(mystat.st_mode))
  {
    putc('d');
  }
  else
  {
    putc('l');
  }

  for (int i = 8; i >= 0; i--)
  {
    if (mystat.st_mode & (1 << i))
    {
      putc(t1[i]);
    }
    else
    {
      putc(t2[i]);
    }
  }

  // output info
  printf(" %d ", mystat.st_nlink);
  printf("%d ", mystat.st_gid);
  printf("%d ", mystat.st_uid);
  printf("%d ", mystat.st_size);
  printf("%s\n", fname);
  // print link name if file is link type
  if (S_ISLNK(mystat.st_mode))
  {
    char lname[128];
    printf(" -> ");
    readlink(fname, lname);
    prints(lname);
  }

  printf("\n");
  //printf("\nfname=%s\nst_mode=%d\n", fname, mystat.st_mode);
}

int ls_dir(char *dname)
{
  DIR *dp;
  char *cp, buf[1024], fname[128];
  bzero(buf, 1024); bzero(fname, 128);
  int dir_fd = open(dname, O_RDONLY);
  read(dir_fd, buf, 1024);
  cp = buf;
  dp = (DIR*)buf;

  while (cp < 1024 + buf)
  {
    strncpy(fname, dp->name, dp->name_len);
    ls_file(fname);

    cp += dp->rec_len;
    dp = (DIR*)cp;
  }
}
