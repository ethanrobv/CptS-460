#include "type.h"
#include "keymap2"

extern int color;

#define KCNTL 0x00
#define KSTAT 0x04
#define KDATA 0x08
#define KCLK  0x0C
#define KISTA 0x10

typedef volatile struct kbd{
  char *base;
  char buf[128];
  int head, tail, data, room;
}KBD;

volatile KBD kbd;

int release;       // key release flag

int kbd_init()
{
  KBD *kp = &kbd;
  kp->base = (char *)0x10006000;
  *(kp->base + KCNTL) = 0x10;    // bit4=Enable bit0=INT on
  *(kp->base + KCLK)  = 8;       // ARM manual says clock=8
  kp->head = kp->tail = 0;       // circular buffer char buf[128]
  kp->data = 0; kp->room = 128;

  release = 0;
}

int capitalize;
int ctrl;
void kbd_handler()
{
  u8 scode, c;
  KBD *kp = &kbd;

  color = YELLOW;

  scode = *(kp->base + KDATA);   // get scan code of this interrpt

  kputs("kbd interrupt scancode = "); kprintx(scode);

  //if shift key *PRESSED*
  if (scode == 0x12 || scode == 0x59)
  {
    if (release == 1)
    {
      capitalize = 0;
      release = 0;
      return;
    }
    else
    {
      capitalize = 1;
      return;
    }
  }

  //if ctrl key *PRESSED*
  else if (scode == 0x14)
  {
    if (release == 1)
    {
      ctrl = 0;
      release = 0;
      return;
    }
    else
    {
      ctrl = 1;
      return;
    }
  }

  if (release == 1)
  {
    release = 0;
    return;
  }

  if (scode == 0xF0)
  {
    release = 1;
    return;
  }

  if (capitalize)
  {
    c = utab[scode];
  }
  // map scode to ASCII in lowercase 
  else
  {
    c = ltab[scode];
  }

  if (ctrl)
  {
    if (c == 'c' || c == 'C')
    {
      kputs("kbd interrupt : Ctrl + C key\n");
    }
    if (c == 'd' || c == 'D')
    {
      kputs("kbd interrupt : ");
      c = 0x4;
      kputc(c);
      kputs("(EOF)\n");
    }
  }
  else
  {
    kputs("kbd interrupt : "); kputc(c); kputs("\n");
  }

  kp->buf[kp->head++] = c;
  kp->head %= 128;
  kp->data++; kp->room--;
}

int kgetc()
{
  char c;
  KBD *kp = &kbd;

  unlock();                          // unmask IRQ in case it was masked out
  while(kp->data == 0);              // BUSY wait while kp->data is 0 

  lock();                            // mask out IRQ
    c = kp->buf[kp->tail++]; 
    kp->tail %= 128;                 /*** Critical Region ***/
    kp->data--; kp->room++;
    unlock();                        // unmask IRQ
  return c;
}

int kgets(char s[ ])
{
  char c;
  while( (c = kgetc()) != '\r'){
    *s = c;
    s++;
  }
  *s = 0;
}

