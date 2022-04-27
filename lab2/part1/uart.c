#define UDR 0x00
#define UFR 0x18

typedef struct uart{  
  char *base;           // base address
  int  n;
}UART;

UART uart[4];          // 4 UART structs

// For versatile_epb : uarts are at 0x101F1000, 2000, 3000; 10009000 

int uart_init()
{
  int i; UART *up;
  for (i=0; i<4; i++){
    up = &uart[i];
    up->base = (char *)(0x101f1000 + i*0x1000);
    up->n = i;
  }
  uart[3].base = (char *)(0x10009000);
}


/******************************
Flag register (offset 0x18): status of UART port
   7    6    5    4    3    2   1    0
| TXFE RXFF TXFF RXFE BUSY  -   -    -|

where TXFE=Tx Empty, RXFF=Rx Full, TXFF=Tx Full, RXFE=Rx Empty
*****************************/

int ugetc(UART *up)
{
  while ( *(up->base + UFR) & 0x10 );  // while UFR.bi4=1 (RxEmpty: no data);
  return (char)(*(up->base + UDR));    // return *UDR
}

int uputc(UART *up, char c)
{
  while ( *(up->base + UFR) & 0x20 );  // while UFR.bit5=1 (TxFull: not empty);
  *(up->base + UDR) = (int)c;          // write c to UDR
}

int ugets(UART *up, char *s)
{
  while ((*s = (char)ugetc(up)) != '\r'){
    uputc(up, *s);
    s++;
  }
 *s = 0;
}

int uprints(UART *up, char *s)
{
  while(*s)
    uputc(up, *s++);
}

char *tab = "0123456789ABCDEF";
int rpu(UART *up, unsigned int x, int base)
{
  char c;
  if (x)
  {
    c = tab[x % base];
    rpu(up, x / base, base);
    uputc(up, c);
  }
}

void uprintsigned(UART *up, int x, int base)
{
  if (base == 16)
    uprints(up, "0x");
	if (x < 1)
	{
    uputc(up, '-');	
	  (x==0)? uputc(up, '0') : rpu(up, x * -1, base);
  }
  else
  {
    (x==0)? uputc(up, '0') : rpu(up, x, base);
  }
}

int uprintu(UART *up, unsigned int x, int base)
{
  if (x == 0)
  {
    uputc(up, '0');
  }
  else
  {
    rpu(up, x, base);
  }
  uputc(up, ',');
}

int uprintf(UART *up, char *fmt, ...)
{
  char *cp = fmt;
  int *ip = (int*)&fmt + 1;
  while (*cp != '\0')
  {
    if (*cp == '%')
    {
      cp++;
      switch (*cp)
      {
      case 'c':
        uputc(up, (char)*ip);
        ip++;
        cp++;
        break;
      
      case 's':
        uprints(up, (char*)ip);
        ip++;
        cp++;
        break;
      
      case 'd':
        uprintsigned(up, (int)*ip, 10);
        ip++;
        cp++;
        break;
      
      case 'x':
        uprintsigned(up, (int)*ip, 16);
        ip++;
        cp++;
        break;
      
      case 'u':
        uprintu(up, (unsigned int)*ip, 10);
        ip++;
        cp++;
        break;

      default:
        cp++;
        break;
      }
    }

    else if (*cp == '\n')
    {
      uputc(up, '\n');
      break;
    }

    else if (*cp == '\r')
    {
      uputc(up, '\r');
      break;
    }
    
    else
    {
      uputc(up, *cp);
      cp++;
    }
  }
}