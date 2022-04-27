/**************** t.c file **************/
#include "uart.c"

int v[10] = {1,2,3,4,5,6,7,8,9,10};
int sum;

int main()
{
  int i;
  char line[128];
  UART *up;
  uart_init();
  up = &uart[0];
  uprintf(up, "UART initiatlized\n\r");
  while (1)
  {
    uprintf(up, "enter a line : ");
    ugets(up, line);
    if (line[0]==0)
      break;
    uprintf(up, "  line = %s \n\r");
    //uprints(up, "  line = "); uprints(up, line); uprints(up, "\n\r");
  }
  
  uprintf(up, "Compute sum of array\n\r");
  sum = 0;
  for (i=0; i<10; i++)
    sum += v[i];
  uprintf(up, "sum in hex = %x\n\r", sum);
  uprintf(up, "sum in decimal = %d \n\rEND OF UART DEMO\n\r", sum);
  //uprints(up, "sum = ");
  //uputc(up, (sum/10)+'0'); uputc(up, (sum%10)+'0');
  //uprints(up, "\n\r");
  
  //uprints(up, "END OF UART DEMO\n\r");
  return 0;
}
