#ifndef TYPE_H
#define TYPE_H
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

#define UART0_BASE_ADDR 0x101f1000
#define UART0_DR   (*((volatile u32 *)(UART0_BASE_ADDR + 0x000)))
#define UART0_FR   (*((volatile u32 *)(UART0_BASE_ADDR + 0x018)))
#define UART0_IMSC (*((volatile u32 *)(UART0_BASE_ADDR + 0x038)))

#define UART1_BASE_ADDR 0x101f2000
#define UART1_DR   (*((volatile u32 *)(UART1_BASE_ADDR + 0x000)))
#define UART1_FR   (*((volatile u32 *)(UART1_BASE_ADDR + 0x018)))
#define UART1_IMSC (*((volatile u32 *)(UART1_BASE_ADDR + 0x038)))

#define KBD_BASE_ADDR 0x10006000
#define KBD_CR (*((volatile u32 *)(KBD_BASE_ADDR + 0x000)))
#define KBD_DR (*((volatile u32 *)(KBD_BASE_ADDR + 0x008)))

#define TIMER0_BASE_ADDR 0x101E2000
#define TIMER0_LR (*((volatile u32 *)(UART0_BASE_ADDR + 0x000)))
#define TIMER0_BR (*((volatile u32 *)(UART0_BASE_ADDR + 0x032)))

#define VIC_BASE_ADDR 0x10140000
#define VIC_STATUS    (*((volatile u32 *)(VIC_BASE_ADDR + 0x000)))
#define VIC_INTENABLE (*((volatile u32 *)(VIC_BASE_ADDR + 0x010)))
#define VIC_VADDR     (*((volatile u32 *)(VIC_BASE_ADDR + 0x030)))

#define SIC_BASE_ADDR 0x10003000
#define SIC_STATUS    (*((volatile u32 *)(SIC_BASE_ADDR + 0x000)))
#define SIC_INTENABLE (*((volatile u32 *)(SIC_BASE_ADDR + 0x008)))
#define SIC_ENSET     (*((volatile u32 *)(SIC_BASE_ADDR + 0x008)))
#define SIC_PICENSET  (*((volatile u32 *)(SIC_BASE_ADDR + 0x020)))

#define BLUE   0
#define RED    1
#define GREEN  2
#define CYAN   3
#define YELLOW 4
#define PURPLE 5
#define WHITE  6

#define NPROC     9 
#define SSIZE 1024

#define FREE   0
#define READY  1
#define SLEEP  2
#define BLOCK  3
#define ZOMBIE 4
#define PAUSE  5
#define printf  kprintf

struct semaphore{
  int value;
  struct proc *queue;
};

typedef struct mbuf{
  struct mbuf *next;
  int pri;
  int pid;
  char contents[64];
}MBUF;
 
typedef struct proc{
  struct proc *next;
  int    *ksp;
  int    status;
  int    pid;

  int    priority;
 
  int    ppid;
  struct proc *parent;
  int    event;
  int    exitCode;
  int    pause; 

  MBUF *mqueue;
  struct semaphore nmsg;
  struct semaphore mlock;


  int    kstack[SSIZE];
}PROC;

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;


#endif