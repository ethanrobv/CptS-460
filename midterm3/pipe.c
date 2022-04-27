// pipe.c file
#include "type.h"

PIPE pipe[NPIPE];

int pipe_init()
{
  // fill in code
  PIPE *p;
  for (int i = 0; i < NPIPE; i++)
  {
    p = &pipe[i];
    p->head = 0;
    p->tail = 0;
    p->data = 0;
    p->room = PSIZE;
    p->status = 0;
    p->nreader = 0;
    p->nwriter = 0;
  }
}

int show_pipe()
{
  PIPE *p = &pipe[0];
  int i;
  printf("----------------------------------------\n");
  printf("room=%d data=%d buf=", p->room, p->data);
  for (i = 0; i < p->data; i++)
    kputc(p->buf[p->tail + i]);
  printf("\n");
  printf("----------------------------------------\n");
}

PIPE *create_pipe()
{
  // fill in code
  for (int i = 0; i < NPIPE; i++)
  {
    if (pipe[i].status == 0)
    {
      return &pipe[i];
    }
  }
  printf("NO MORE PIPES!!!\n");
  return -1;
}

int read_pipe(PIPE *p, char *buf, int n)
{
  // fill in code;
  // add code to handle: no data AND no writer: return 0
  /*
  running->pipe = p;
  running->readWriteNone = 0;
  p->nreader++;
  */
  
  if (p->nwriter == 0 && n <= 0)
  {
    printf("BROKEN PIPE - NO DATA && NO WRITER\n");
    return 0;
  }

  int ret;
  char c;

  if (n <= 0)
    return 0;
  show_pipe();

  while (n)
  {
    printf("reader %d reading pipe\n", running->pid);
    ret = 0;
    while (p->data)
    {
      *buf = p->buf[p->tail++];
      p->tail %= PSIZE;
      buf++;
      ret++;
      p->data--;
      p->room++;
      n--;
      if (n <= 0)
        break;
    }
    show_pipe();
    if (ret)
    { /* has read something */
      kwakeup(&p->room);
      return ret;
    }
    // pipe has no data
    printf("reader %d sleep for data\n", running->pid);
    kwakeup(&p->room);
    ksleep(&p->data);
    continue;
  }
}

int write_pipe(PIPE *p, char *buf, int n)
{
  /*
  running->pipe = p;
  running->readWriteNone = 1;
  p->nwriter++;
  */
  // fill in code;
  // add code to detect BROKEN pipe: print BROKEN pipe message, then exit
  
  if (p->nreader == 0)
  {
    printf("BROKEN PIPE - NO MORE READERS\n");
    kexit(running->status);
  }
  
  char c;
  int ret = 0;
  show_pipe();
  while (n)
  {
    printf("writer %d writing pipe\n", running->pid);
    printf("pipe->room=%d\n", p->room);
    while (p->room)
    {
      p->buf[p->head++] = *buf;
      p->head %= PSIZE;
      buf++;
      ret++;
      p->data++;
      p->room--;
      n--;
      if (n <= 0)
      {
        show_pipe();
        kwakeup(&p->data);
        return ret;
      }
    }
    // we have bytes to write but no more room in pipe
    show_pipe();
    printf("writer %d sleep for room\n", running->pid);
    kwakeup(&p->data);
    ksleep(&p->room);
  }
}
