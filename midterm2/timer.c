// timer.c file
#include "type.h"

/***** timer confiuguration values *****/
#define CTL_ENABLE (0x00000080)
#define CTL_MODE (0x00000040)
#define CTL_INTR (0x00000020)
#define CTL_PRESCALE_1 (0x00000008)
#define CTL_PRESCALE_2 (0x00000004)
#define CTL_CTRLEN (0x00000002)
#define CTL_ONESHOT (0x00000001)

// timer register offsets from base address
/**** byte offsets *******
#define TLOAD   0x00
#define TVALUE  0x04
#define TCNTL   0x08
#define TINTCLR 0x0C
#define TRIS    0x10
#define TMIS    0x14
#define TBGLOAD 0x18
*************************/
/** u32 * offsets *****/
#define TLOAD 0x0
#define TVALUE 0x1
#define TCNTL 0x2
#define TINTCLR 0x3
#define TRIS 0x4
#define TMIS 0x5
#define TBGLOAD 0x6

typedef volatile struct timer
{
  u32 *base;            // timer's base address; as u32 pointer
  int tick, hh, mm, ss; // per timer data area
  char clock[16];
} TIMER;

volatile TIMER timer[4]; // 4 timers; 2 timers per unit; at 0x00 and 0x20

int k;

typedef struct tq
{
  struct tq *next;
  int time;
  PROC *proc;
  int (*action)(int);
} TQE;
TQE *tq, tqe[NPROC];

int inittq()
{
  for (int i = 0; i < NPROC; i++)
  {
    tqe[i].next = 0;
    tqe[i].time = -1;
    tqe[i].proc = 0;
    tqe[i].action = 0;
  }
  tq = 0;
}

int itimer(int t)
{
  if (running->pid == 1 && running->child == 0)
  {
    printf("Error, PROC 1 has no children\n");
    return -1;
  }
  tqe[running->pid].time = t;
  tqe[running->pid].proc = running;
  tqe[running->pid].action = kwakeup;

  lock();
  if (tq == 0)
  {
    tq = &tqe[running->pid];
  }
  else
  {
    TQE *next = tq, *prev = 0;
    while (next != 0 && tqe[running->pid].time - next->time > 0)
    {
      tqe[running->pid].time -= next->time;
      prev = next;
      next = next->next;
    }
    if (next == 0)
    {
      prev->next = &tqe[running->pid];
    }
    else
    {
      if (prev != 0)
      {
        tqe[running->pid].next = prev->next;
        prev->next = &tqe[running->pid];
      }
      else
      {
        if (tq->time > tqe[running->pid].time)
          tq->time -= tqe[running->pid].time;
        tqe[running->pid].next = tq;
        tq = &tqe[running->pid];
      }
    }
  }
  ksleep((int)tqe[running->pid].proc);

  unlock();
}

void printTimeQueue()
{
  lock();
  TQE *temp = tq;
  printf("TIME QUEUE: ");
  while (temp != 0)
  {
    printf("[time=%d, proc=%d] -> ", temp->time, temp->proc->pid);
    temp = temp->next;
  }
  printf("null\n");
  unlock();
}

int checkTimeQueue()
{
  lock();
  
  if (tq->time == 0)
  {
    TQE *temp = tq;
    tq = tq->next;
    temp->next = 0;
    temp->time = -1;
    temp->action((int)temp->proc);
    temp->action = 0;
    temp->proc = 0;
  }
  tq->time--;

  //printTimeQueue();
  unlock();
}


void timer_init()
{
  int i;
  TIMER *tp;
  printf("timer_init()\n");

  for (i = 0; i < 4; i++)
  {
    tp = &timer[i];
    if (i == 0)
      tp->base = (u32 *)0x101E2000;
    if (i == 1)
      tp->base = (u32 *)0x101E2020;
    if (i == 2)
      tp->base = (u32 *)0x101E3000;
    if (i == 3)
      tp->base = (u32 *)0x101E3020;

    *(tp->base + TLOAD) = 0x0; // reset
    *(tp->base + TVALUE) = 0x0;
    *(tp->base + TRIS) = 0x0;
    *(tp->base + TMIS) = 0x0;
    *(tp->base + TCNTL) = 0x62; // 011-0000=|En|Pe|IntE|-|scal=00|32-bit|0=wrap|
    *(tp->base + TBGLOAD) = 0xE0000 / 60;

    tp->tick = tp->hh = tp->mm = tp->ss = 0;
    // strcpy((char *)tp->clock, "00:00:00");
    char *src = (char *)tp->clock;
    char *dest = "00:00:00\0";
    while (*src)
    {
      *dest++ = *src++;
    }
    *dest = 0;
  }
}

void timer_handler(int n)
{
  u16 i;
  u8 flag = 0;
  TIMER *t = &timer[n];

  // increment the timer's second (and potentially minute and hour fields) each second (when tick gets to 60)
  // switch off clearing and printing cursor every 60 ticks (1 second)
  t->tick++;
  if (t->tick % 60 == 0)
  {
    //if (t->tick == 30 || t->tick == 90)
      //clrcursor();
    //if (t->tick == 60 || t->tick == 120)
    //{
      //putcursor();
      //t->tick = 0;
    //}
    if (t->tick % 60 == 0)
    {
      t->tick = 0;
      t->ss++;
      if (t->ss >= 60)
      {
        t->ss = 0;
        t->mm++;
        if (t->mm >= 60)
        {
          t->mm = 0;
          t->hh++;
        }
      }
      checkTimeQueue();
    }
    // printf("timer interrupt\n");
  }

  // clear and re-print clock every second
  if (t->tick % 60 == 0)
  {
    for (i = 0; i < 8; i++)
    {
      unkpchar(t->clock[i], 0, 60 + i);
    }
    //  0    1   2    3     4    5    6     7
    // hour hour : minute minute : second second
    // use quotient + remainder from division by 10 to obtain number at each decimal place
    t->clock[0] = '0' + (t->hh / 10);
    t->clock[1] = '0' + (t->hh % 10);
    t->clock[2] = ':';
    t->clock[3] = '0' + (t->mm / 10);
    t->clock[4] = '0' + (t->mm % 10);
    t->clock[5] = ':';
    t->clock[6] = '0' + (t->ss / 10);
    t->clock[7] = '0' + (t->ss % 10);
    for (i = 0; i < 8; i++)
      kpchar(t->clock[i], 0, 60 + i);
  }

  timer_clearInterrupt(n);
}

void timer_start(int n) // timer_start(0), 1, etc.
{
  TIMER *tp = &timer[n];

  *(tp->base + TCNTL) |= 0x80; // set enable bit 7
}

int timer_clearInterrupt(int n) // timer_start(0), 1, etc.
{
  TIMER *tp = &timer[n];
  *(tp->base + TINTCLR) = 0xFFFFFFFF;
}

void timer_stop(int n) // timer_start(0), 1, etc.
{
  TIMER *tp = &timer[n];
  *(tp->base + TCNTL) &= 0x7F; // clear enable bit 7
}
