#include "type.h"
extern PROC *running;
extern PROC *readyQueue;

int P(struct semaphore *s)
{
  // fill in code
  int sr = int_off();
  s->value--;
  if (s->value < 0)
  {
    running->status = BLOCK;
    enqueue(&s->queue, running);
    tswitch();
  }
  int_on(sr);
}

int V(struct semaphore *s)
{
  // fill in code
  int sr = int_off();
  s->value++;
  if (s->value <= 0)
  {
    PROC *p = dequeue(&s->queue);
    p->priority = 1;
    enqueue(&readyQueue, p);
  }
  int_on(sr);
}
