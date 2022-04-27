#include "type.h"

int enqueue();
int tswitch();
int int_on();
int int_off();
extern PROC proc[NPROC];
extern PROC *sleepList;
extern PROC *readyQueue;
extern PROC *freeList;
extern PROC *running;

int kwait(int *status)
{
  if (running->child == 0)
  {
    printf("PROC %d has no children.\n", running->pid);
    return -1;
  }

  PROC *temp = running->child;
  // if first child is ZOMBIE
  if (temp->status == ZOMBIE)
  {
    running->child = temp->sibling;
    // copy exit val to *status
    *status = temp->exitCode;
    // free the ZOMBIE process
    enqueue(&freeList, temp);
    temp->status = FREE;
    return temp->pid;
  }

  // search for ZOMBIE children
  PROC *prev = temp;
  temp = temp->sibling;
  while (temp != 0)
  {
    if (temp->status == ZOMBIE)
    {
      prev->sibling = temp->sibling;
      *status = temp->exitCode;
      enqueue(&freeList, temp);
      return temp->pid;
    }

    // put PROC to sleep on its own PROC address until a ZOMBIE child is found
    ksleep((int)running);
    prev = temp;
    temp = temp->sibling;
  }
}

int ksleep(int event)
{
  // disable IRQ and return CPSR
  int sr = int_off();
  printf("proc id %d: sleep request, event=%d\n", running->pid, event);
  running->event = event;
  running->status = SLEEP;
  // place running PROC into queue of asleep procs
  enqueue(&sleepList, running);
  printList("Sleep List", sleepList);
  // switch processes
  tswitch();
  // restore CPSR
  int_on(sr);
}

int kwakeup(int event)
{
  // disable IRQ and return CPSR
  int sr = int_off();

  // search sleepList for PROC with event == the function argument
  PROC *temp = 0, *p = dequeue(&sleepList);
  while (p != 0)
  {
    printf("pevent=%d; event=%d\n", p->event, event);
    if (p->status == SLEEP && p->event == event)
    {
      p->status = READY;
      enqueue(&readyQueue, p);
      printf("PROC %d woke up\n", p->pid);
    }
    // requeue every other process in temp queue
    else
    {
      enqueue(&temp, p);
    }

    p = dequeue(&sleepList);
  }

  sleepList = temp;
  printList("Sleep List", sleepList);
  // re-enable IRQ interrupts
  int_on(sr);
}

int kexit(int exitValue)
{
  /*
  printf("proc %d in kexit(), value=%d\n", running->pid, exitValue);
  running->exitCode = exitValue;
  running->status = ZOMBIE;
  kwakeup(running->parent);
  tswitch();
  */
 // don't let proc 1 exit
  if (running->pid == 1)
  {
    printf("PROC 1 can not exit\n");
    return -1;
  }

  // hand off proc's children to parent (or proc 1)
  if (running->child != 0)
  {
    PROC *temp = running->child, *temp2 = running->parent->child;
    // traverse to end of running->parent's child's sibling list
    while (temp2->sibling != 0)
    {
      temp2 = temp2->sibling;
    }
    
    // foreach of running PROC's children, append to end of running->parent's child's sibling list
    while (temp != 0)
    {
      temp->parent = running->parent;
      temp->ppid = running->ppid;
      temp2->sibling = temp;
      temp2 = temp2->sibling;
      temp = temp->sibling;
    }
  }

  running->exitCode = exitValue;
  running->status = ZOMBIE;
  running->priority = 0;
  // wakeup parent
  kwakeup((int)running->parent);
  tswitch();
}