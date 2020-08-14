#include "CController.h"

/*******************************************************************/
CDMA::CDMA()
{
  this->configDMAfor_TXUARTs();
}
/*******************************************************************/
CDMA::~CDMA()
{}
/*******************************************************************/
void CDMA::configDMAfor_TXUARTs(void)
{
  DMA0CTL&=~DMAIE;
  DMA1CTL&=~DMAIE;
  DMA2CTL&=~DMAIE;

  DMA0SZ=0x0000;
  DMA1SZ=0x0000;
  DMACTL0=DMA0TSEL_4|DMA2TSEL_10 ; // UART 0 TX = Channel 0; UART1 TX = Cnannel 2  trigger source for DMA0 and DMA1
  DMACTL1=0x0000;

  DMA0DA=(unsigned short)(&U0TXBUF);
  DMA0CTL= DMADT_0|DMADSTINCR_0|DMASRCINCR_3|DMASBDB|DMAIE;

  DMA1CTL&= ~DMAIE;

  DMA2DA=(unsigned short)(&U1TXBUF);
  DMA2CTL= DMADT_0|DMADSTINCR_0|DMASRCINCR_3|DMASBDB|DMAIE;

}
/*******************************************************************/
void CDMA::configDMAfor_TXRX_UART_SDMem(void)
{
  DMA0CTL&=~DMAIE;
  DMA1CTL&=~DMAIE;
  DMA2CTL&=~DMAIE;

  DMA0SZ=0x0000;
  DMA1SZ=0x0000;
  DMA2SZ=0x0000;

  DMACTL0=DMA0TSEL_4|DMA1TSEL_10|DMA2TSEL_10 ; // UART 0 TX = Channel 0; UART1 RX = Cnannel 1; UART1 RX = Channel 2  trigger source for DMA0, DMA1 and DMA2
  DMACTL1=0x0000;

  DMA0DA=(unsigned short)(&U0TXBUF);
  DMA0CTL= DMADT_0|DMADSTINCR_0|DMASRCINCR_3|DMASBDB|DMAIE;

  DMA1SA=(unsigned short)(&U1RXBUF);                        // To Receive data
  DMA1CTL= DMADT_0|DMADSTINCR_3|DMASRCINCR_0|DMASBDB|DMAIE;

  DMA2DA=(unsigned short)(&U1TXBUF);                        // To send and receive data
  DMA2CTL= DMADT_0|DMADSTINCR_0|DMASRCINCR_0|DMASBDB|DMAIE;
}
/*******************************************************************/
void CDMA::configDMAfor_TX_UART_SDMem(void)
{
  DMA0CTL&=~DMAIE;
  DMA1CTL&=~DMAIE;
  DMA2CTL&=~DMAIE;

  DMA0SZ=0x0000;
  DMA1SZ=0x0000;
  DMA2SZ=0x0000;
  //DMACTL0=DMA0TSEL_4|DMA2TSEL_10 ; // UART 0 TX = Channel 0; UART1 TX = Cnannel 1  trigger source for DMA0 and DMA1
  DMACTL1=0x0000;

  DMA0DA=(unsigned short)(&U0TXBUF);
  DMA0CTL= DMADT_0|DMADSTINCR_0|DMASRCINCR_3|DMASBDB|DMAIE;

  DMA1CTL&=~DMAIE;

  DMA2DA=(unsigned short)(&U1TXBUF);                        // To send and receive data
  DMA2CTL= DMADT_0|DMADSTINCR_0|DMASRCINCR_3|DMASBDB|DMAIE;
}
/*******************************************************************/
void CDMA::setDMASourceAddr_CH0(char* Buffer)
{
  DMA0SA=(unsigned short)Buffer;
}
/*******************************************************************/
void CDMA::setDMASourceAddr_CH1(char* Buffer)
{
  DMA1SA=(unsigned short)Buffer;
}
/*******************************************************************/
void CDMA::setDMADestinyAddr_CH1(char* Buffer)
{
  DMA1DA=(unsigned short)Buffer;
}
/*******************************************************************/
void CDMA::setDMASourceAddr_CH2(char* Buffer)
{
  DMA2SA=(unsigned short)Buffer;
}
/*******************************************************************/
void CDMA::setDMADestinyAddr_CH2(char* Buffer)
{
  DMA2DA=(unsigned short)Buffer;
}
/*******************************************************************/
void CDMA::setTx0_Size(int size)
{
  DMA0SZ=size;
}
/*******************************************************************/
void CDMA::setTx1_Size(int size)
{
  DMA1SZ=size;
}
/*******************************************************************/
void CDMA::setRx1_Size(int size)
{
  DMA1SZ=size;
}
/*******************************************************************/
void CDMA::setTx2_Size(int size)
{
  DMA2SZ=size;
}
/*******************************************************************/
void CDMA::setRx2_Size(int size)
{
  DMA2SZ=size;
}
/*******************************************************************/
void CDMA::setDMA_CH0_EN()
{
  DMA0CTL|=DMAEN;
}
/*******************************************************************/
void CDMA::setDMA_CH1_EN()
{
  DMA1CTL|=DMAEN;
}
/*******************************************************************/
void CDMA::setDMA_CH2_EN()
{
  DMA2CTL|=DMAEN;
}
/*******************************************************************/
void CDMA::setDMA_CH0_REQ()
{
  DMA0CTL|=DMAREQ;
}
/*******************************************************************/
void CDMA::setDMA_CH1_REQ()
{
  DMA1CTL|=DMAREQ;
}
/*******************************************************************/
void CDMA::setDMA_CH2_REQ()
{
  DMA2CTL|=DMAREQ;
}
/*******************************************************************/
