// wait.c file: ksleep(), kwakeup(), kexit()
#include "type.h"

extern PROC *running;
extern PROC *readyQueue;
extern PROC *sleepList;

int ksleep(int event)
{
  // your ksleep()
  int sr = int_off(); // mask interrupts
  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  printList(sleepList);
  tswitch();
  int_on(sr);
}

int kwakeup(int event)
{
  // your kwakeup()
  PROC *temp = 0, *p;
  int sr = int_off();

  while ((p = dequeue(&sleepList)) != 0)
  {
    if (p->event == event)
    {
      p->status = READY;
      enqueue(&readyQueue, p);
      printf("PROC %d WOKE UP\n", p->pid);
    }
    else
    {
      enqueue(&temp, p);
    }
  }
  sleepList = temp;
  int_on(sr);
}

int kexit(int exitCode)
{
  running->exitCode = exitCode;
  running->status = ZOMBIE;
  running->priority = 0;
  // decrement nwriter or nreader on pipe
  switch (running->readWriteNone)
  {
  case 0:
    running->pipe->nreader--;
    break;
  
  case 1:
    running->pipe->nwriter--;
    break;
  
  default:
    break;
  }
  running->pipe = 0;
  running->readWriteNone = -1;
  kwakeup(running->parent);
  tswitch();
}
