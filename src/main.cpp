#include "xil_printf.h"
#include "xparameters.h"
#include "xuartpsv.h"
#include <stdio.h>
#include <string>

int main(void) {
  xil_printf("\r\n");
  xil_printf("========================================\r\n");
  xil_printf("  VD100 Bare-Metal UART Echo            \r\n");
  xil_printf("  XCVE2302 | PS UART0 | 115200 baud     \r\n");
  xil_printf("========================================\r\n");
  xil_printf("Type anything, it will be echoed back...\r\n\r\n");

  xil_printf("[Type] >> ");
  std::string text;
  while (1) {
    while (!XUartPsv_IsReceiveData(XPAR_XUARTPSV_0_BASEADDR))
      ;

    u8 c = XUartPsv_RecvByte(XPAR_XUARTPSV_0_BASEADDR);

    if (c == '\r') {
      if (!text.empty()) {
        xil_printf("\r\n[VD100] >> %s\r\n[Type]  >> ", text.c_str());
        text = "";
      }
    } else if (c == 0x7F || c == 0x08) {
      // Backspace — remove last char and clear from terminal
      if (!text.empty()) {
        text.pop_back();
        xil_printf("\b \b"); // erase character on terminal
      }
    } else {
      text += (char)c;
      XUartPsv_SendByte(XPAR_XUARTPSV_0_BASEADDR, c); // local echo while typing
    }
  }

  return 0;
}