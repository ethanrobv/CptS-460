/************** login.c file ***************/
/*
 * login.c - Upon entry, argv[0]=login, argv[1]=/dev/ttyX
 */

#include "ucode.c"

#define MAX_ACCOUNTS 16
#define MAX_ACCOUNT_REC_LENGTH 128

int in, out, err;
char username[128], password[128];
int num_accounts;
char buf[1024], debug[128];

// unreasonable for someone to have a longer account entry
char account_records[MAX_ACCOUNTS][MAX_ACCOUNT_REC_LENGTH];

// slices passwd file up into account_records array
int passwd_tok(char *str);

// checks user inputted password and username against account record
int verify_login(char *pwattempt, char *unattempt);

int get_user_stuff(int user_number, int field_index, char *dest);

/*
    1) close file desc 0,1 inherited from init
    2) open argv[1] 3 times as in(0), out(1), err(2)
    3) set tty(argv[1]); // set tty name string in PROC.tty
    4) open /etc/passwd file for READ;
    5)  while(1)
        {
          printf("login");  gets(name);
          printf("password:"); gets(password);
          for each line in /etc/passwd file do:
          {
            tokenize user account line;
    6)      if (user has a valid account)
            {
    7)        change uid, gid, to user's uid, gid;  // chuid()
              change cwd to user's home DIR;  // chdir()
              close opened /etc/passwd file;  // close()
    8)        exec to program in user account // exec()
            }
          }
          printf("login failed, try again\n");
        }
*/

int main(int argc, char *argv[])
{
  char h_dir[64], s_dir[64], convert_str[16];
  int gid, uid;
  int i;
  int passwd_fd;
  STAT mystat;
  STAT *mystat_ptr = &mystat;

  printf("this is login\n");

  for (i = 0; i < argc; i++)
  {
    printf("argv[%d] = %s\n", i, argv[i]);
  }

  // STEP 1:
  close(0);
  close(1);

  // STEP 2:
  in = open(argv[1], O_RDONLY);
  out = open(argv[1], O_WRONLY);
  err = open(argv[1], O_RDWR);

  // STEP 3:
  settty(argv[1]);

  // STEP 4:
  passwd_fd = open("/etc/passwd", O_RDONLY);

  // STEP 5:
  while (1)
  {
    // clear screen with regex
    // printf("\e[1;1H\e[2J");

    printf("\nUSERNAME:");
    gets(username);
    printf("PASSWORD:");
    gets(password);

    printf("\nAttempting to log you in...\n");
    // get passwd file size
    stat("/etc/passwd", mystat_ptr);

    read(passwd_fd, buf, mystat.st_size);
    
    passwd_tok(buf);

    // verify login
    int good_login = verify_login(password, username);
    if (good_login > 0)
    {
      // change uid, gid
      bzero(h_dir, 64); bzero(s_dir, 64); bzero(convert_str, 16);
      get_user_stuff(good_login, 2, convert_str);
      uid = atoi(convert_str);
      bzero(convert_str, 16);
      get_user_stuff(good_login, 3, convert_str);
      gid = atoi(convert_str);
      chuid(uid, gid);

      // chdir to user's home dir
      get_user_stuff(good_login, 5, h_dir);
      get_user_stuff(good_login, 6, s_dir);
      
      chdir(h_dir);

      // close passwd file
      close(passwd_fd);

      // exec user's shell
      exec(s_dir);
    }
  }
}

int passwd_tok(char *str)
{
  int i;
  for (i = 0; i < MAX_ACCOUNTS; i++)
  {
    bzero(account_records[i], MAX_ACCOUNT_REC_LENGTH);
  }

  num_accounts = 0;
  int j = 0;
  for (i = 0; i < strlen(str); i++)
  {
    if (str[i] == EOF)
    {
      return 0;
    }

    if (str[i] == '\n' || str[i] == '\r')
    {
      if (str[i + 1] == EOF)
      {
        return 0;
      }
      account_records[num_accounts][j] = '\0';
      num_accounts++;
      j = 0;
    }
    else if (str[i] == ':')
    {
      account_records[num_accounts][j] = ':';
      j++;
    }
    else
    {
      account_records[num_accounts][j] = str[i];
      j++;
    }
  }
  account_records[num_accounts][j] = '\0';
}

int verify_login(char *pwattempt, char *unattempt)
{
  char real_password[64], real_username[64];
  int field_index;
  for (int i = 0; i < num_accounts; i++)
  {
    for (int j = 0; account_records[i][j] != '\0'; j++)
    {
      int k = 0;
      while (account_records[i][j] != ':')  // GET USER NAME FROM RECORD
      {
        real_username[k] = account_records[i][j];
        k++;
        j++;
      }
      real_username[k] = '\0';
      k = 0;
      j++;

      while (account_records[i][j] != ':')  // GET PASSWORD FROM RECORD
      {
        real_password[k] = account_records[i][j];
        k++;
        j++;
      }
      real_password[k] = '\0';

      if (!strcmp(real_username, unattempt))
      {
        if (!strcmp(real_password, pwattempt))
        {
          return i;
        }
      }

      break;
    }
  }

  return -1;
}

int get_user_stuff(int user_number, int field_index, char *dest)
{
  int flag = 0;
  for (int i = 0; i < strlen(account_records[user_number]); i++)
  {
    if (flag == field_index)
    {
      int j = 0;
      while (account_records[user_number][i] != ':' && account_records[user_number][i] != '\n' && account_records[user_number][i] != '\0' && account_records[user_number][i] != '\r')
      {
        dest[j] = account_records[user_number][i];
        j++;
        i++;
      }
      dest[j] = '\0';
      return 0;
    }
    if (account_records[user_number][i] == ':')
    {
      flag++;
    }
  }
  return -1;
}