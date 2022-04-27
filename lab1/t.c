/*******************************************************
 *                  @t.c file                          *
 *******************************************************/
#include "ext2.h"

int prints(char *s)
{
  while (*s)
    putc(*s++);
}

int gets(char *s)
{
  while ((*s = getc()) != '\r')
    putc(*s++);

  *s = 0;
}

u16 NSEC = 2;
char buf1[BLK], buf2[BLK];

u16 getblk(u16 blk, char *buf)
{
  readfd((2 * blk) / CYL, ((2 * blk) % CYL) / TRK, ((2 * blk) % CYL) % TRK, buf);
}

u16 search(INODE *ip, char *name)
{
  u16 i;
  char c;
  DIR *dp;
  for (i = 0; i < 12; i++)
  {
    if ((u16)ip->i_block[i])
    {
      getblk((u16)ip->i_block[i], buf2);
      dp = (DIR *)buf2;
      while ((char *)dp < &buf2[BLK])
      {
        c = dp->name[dp->name_len];
        dp->name[dp->name_len] = 0;
        prints(dp->name);
        putc(' ');
        if (!strcmp(dp->name, name))
        {
          prints("\n\r");
          return ((u16)dp->inode);
        }
        dp->name[dp->name_len] = c;
        dp = (char *)dp + dp->rec_len;
      }
    }
  }
  return -1;
}

main()
{
  char *cp, filename[64];
  u16 i, ino, blk, iblk;
  u32 *up;
  GD *gp;
  INODE *ip;
  DIR *dp;
  getblk(2, buf1);
  gp = (GD *)buf1;
  iblk = (u16)gp->bg_inode_table;
  getblk(iblk, buf1);
  ip = (INODE *)buf1 + 1;
  ino = search(ip, "boot")-1;
  if (ino < 0)
    return -1;
  getblk(iblk + (ino / 8), buf1);
  ip = (INODE*)buf1 + (ino % 8);
  ino = search(ip, "mtx")-1;
  if (ino < 0)
    return -1;
  getblk(iblk + (ino / 8), buf1);
  ip = (INODE*)buf1 + (ino % 8);
  if ((u16)ip->i_block[12])
  getblk((u16)ip->i_block[12], buf2);
  setes(0x1000);
  for (i = 0; i < 12; i++)
  {
    getblk((u16)ip->i_block[i], 0);
    putc('*');
    inces();
  }
  if (ip->i_block[12])
  {
    up = (u32 *)buf2;
    while (*up)
    {
      getblk((u16)*up, 0);
      putc('.');
      inces();
      up++;
    }
  }
  prints("go?\n\r");
  getc();
}