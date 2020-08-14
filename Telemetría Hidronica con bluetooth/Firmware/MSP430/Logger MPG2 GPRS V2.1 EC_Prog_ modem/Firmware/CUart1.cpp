#include "stdlib.h"
#include "CController.h"
#include "string"

/*********************************/

/* Constructors */
        CUart1::CUart1()
        {
          this->m_DMA->setDMASourceAddr_CH1((char*)this->Buffer_Tx);
          this->extraTimeout=2;
          this->tries=8;
          this->Buffer_Rx[0]=0;
          //this->configUart(this,1,19200,'S');
          this->configUart(this,1,9600,'A');
          this->closeUart();
          this->openUart(this);
        }

/*********************************************************************/
/* Destructor	*/
	CUart1::~CUart1()
	{}
/*********************************************************************/
/*void CUart0::configUart(CUart0 *Uartx,unsigned int portID,unsigned long bps, char sclk)
{
   Uartx->portID=portID;
   Uartx->bps=bps;
   Uartx->sclk=sclk;
   Uartx->Buffer_Tx_Start=0;
   Uartx->Buffer_Rx[0]=0;
   Uartx->dataPtrRx=0;
   Uartx->dataPtrTx=0;
   Uartx->strRxLen=0;
   Uartx->extraTimeCounter=0;
   Uartx->commTries=0;

   Uartx->statePort=0x00;    //Inicializa el estado del puerto de comunicaciones.

   Uartx->UartError=0x00;

}*/
/*********************************************************************/
/*char CUart0::openUart(CUart0 *Uartx)
{
   if(!isOpen(Uartx))  //Si esta cerrado
   {
      //configWdt('R','R','T','A',0); //Configura el WDT como timer a 1.0 seg
      //this->stopTimeOutByte();

      setBaudRate(Uartx);
      switch (getPortID(Uartx))
      {
      case 0:
             P3OUT&=~BIT3;
             P3DIR&=~BIT3;
             P3SEL|=BIT3;
             UCTL0|=CHAR;
             P3OUT&=~BIT5;
             P3OUT|=BIT4;
             P3SEL|=(BIT4|BIT5);
             P3DIR&=~BIT5;          // Rx
             P3DIR|=BIT4;           // Tx
             ME1|=(URXE0|UTXE0);    // Habilita los módulos de Rx y TX
             UCTL0&=~SWRST;         // Desactiva el estado de Reset la UART
             IE1|=(URXIE0|UTXIE0);  // Habilita interrupciones de la UART.
             break;
      }
      Uartx->statePort|=0x01;  //Establece el puerto como abierto (Ocupado).

      return Uartx->statePort;
   }
   else               //Si esta abierto.
   {return Uartx->statePort&0x01;}
}*/
/*********************************************************************/
/*char CUart0::isOpen(CUart0 *Uartx)
{
 return Uartx->statePort;
}*/
/*********************************************************************/
/*void CUart0::closeUart()
{
  UCTL0|=SWRST;           // Mantiene en estado de Reset la UART
  IE1&=~(URXIE0|UTXIE0); // Deshabilita interrupciones de la UART.
  ME1&=~(URXE0|UTXE0);   // Deshabilita los módulos de Rx y TX

}*/
/*********************************************************************/
/*unsigned int CUart0::getPortID(CUart0 *Uartx)
{
   return Uartx->portID;
}*/
/*********************************************************************/
/*void CUart0::setBaudRate(CUart0 *Uartx)
{
  long int clockFrec;   // Frecuncia de reloj externo de 7.3728 MHz
  ldiv_t calc_rate;

  unsigned int dataBit=0;
  switch(Uartx->sclk)
  {
  case 'U': clockFrec=7372800; P3SEL|=0x008; break;     //UCLK0
  case 'A': dataBit|=SSEL0; break;                      //ACLK
  case 'S': dataBit|=SSEL1; break;                      //SMCLK
  }
  U0TCTL=dataBit;

  calc_rate=ldiv(clockFrec,Uartx->bps);
  U0BR0=calc_rate.quot & 0x0FF;
  U0BR1=(calc_rate.quot / 0x100)&0x0FF;

  double myval=(clockFrec*1000/Uartx->bps);
  myval-=calc_rate.quot*1000;
  calcUARTModulationReg(myval/1000);

}*/
/*********************************************************************/
void CUart1::startTx(char *BufferTx,int strLen)
{
   this->on_RS_232();
   this->statePort|=0x02;
   //Uartx->strTxLen=strLen;
   //Uartx->Buffer_Tx_Start=(unsigned char*)BufferTx;
   //Uartx->dataPtrTx=0;
   this->m_DMA->setDMASourceAddr_CH1((char*)&BufferTx[1]);
   this->m_DMA->setTx1_Size(strLen-1);
   this->m_DMA->setDMA_CH1_EN();
//   this->m_DMA->setDMA_CH1_REQ();
   U1TXBUF=BufferTx[0];
//   IFG2|=UTXIFG1;
//   U1TXBUF=Uartx->Buffer_Tx_Start[Uartx->dataPtrTx];
}
/*********************************************************************/
int CUart1::getBuffRxLen()
{
  return this->strRxLen;
}
/*********************************************************************/
/*void CUart0::clearRxBuffPointer(CUart0 *Uartx)
{
  Uartx->dataPtrRx=0;
}*/
/*********************************************************************/
/*void CUart0::calcUARTModulationReg(double v_remain)
{
  ldiv_t fresult;
  unsigned int resp=0;
  char reg=0x00;
  for(int x=2;x<=9;x++)
  {
    fresult=ldiv(v_remain*x,1);
    reg>>=1;
    if (resp!=fresult.quot)
    {
      reg|=BIT7;
      resp=fresult.quot;
    }
    else
    {
      reg&=~BIT7;
    }
  }
  UMCTL0=reg;
}*/
/*********************************************************************/

CUart1* CUart1::getUART()
{
  return this;
}
/*********************************************************************/
/*
void CUart0::startTimeOutByte()
{
  this->TimeoutByte->configTA('A',0x01,'U'); // Arrancar el Timer
}
*/
/*********************************************************************/
/*void CUart0::stopTimeOutByte()
{
  this->TimeoutByte->configTA('A',0x01,'S'); // Arrancar el Timer
}
*/
/*********************************************************************/

