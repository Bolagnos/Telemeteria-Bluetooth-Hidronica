#include "stdlib.h"
#include "CController.h"
#include "string"

//#define SwUART_0 BIT3     // P3.3

/*********************************/

/* Constructors */
        CUart0::CUart0()
        {
//          P3OUT&=~SwUART_0;       // Establece salida P3.3 a nivel 0
//          P3DIR|=SwUART_0;        // Establece P3.3 como salida
          this->configHandShackPorts();
          this->m_DMA->setDMASourceAddr_CH0((char*)this->Buffer_Tx);
          this->extraTimeout=5;
          this->tries=10;
          this->Buffer_Rx[0]=0;
//          this->configUart(this,0,19200,'S');
          this->portID=0x00;
          this->commTries=0;
          this->configUart(this,0,9600,'A');
          this->closeUart();
          this->openUart(this,false); // Starts with UART comms
        }

/*********************************************************************/
/* Destructor	*/
	CUart0::~CUart0()
	{}
/*********************************************************************/
void CUart0::startTx(char *BufferTx,int strLen)
{
   //this->on_RS_232();
   this->statePort|=0x02;

   if(this->isTXInterruptEnabled())
   {
      this->strTxLen=strLen;
      this->Buffer_Tx_Start=(unsigned char*)BufferTx;
      this->dataPtrTx=0;
      IFG1&=~UTXIFG0;
   }
   else
   {
      this->m_DMA->setDMASourceAddr_CH0((char*)&BufferTx[1]);
      this->m_DMA->setTx0_Size(strLen-1);
      this->m_DMA->setDMA_CH0_EN();
   }
//   this->m_DMA->setDMA_CH0_REQ();
   U0TXBUF=BufferTx[0];
  // IFG1|=UTXIFG0;
//   U0TXBUF=this->Buffer_Tx_Start[this->dataPtrTx];
}
/*********************************************************************/
int CUart0::getBuffRxLen(CUart0 *Uartx)
{
  return Uartx->strRxLen;
}
/*********************************************************************/
CUart0* CUart0::getUART()
{
  return this;
}
/*********************************************************************/
void CUart0::clearBufferRx()
{
  int x;
        for(x=0;x<buffer0_size;x++)    // Borra el contenido del buffer de Rx
        {
          this->Buffer_Rx[x]=0x00;
        }
}
/*********************************************************************/
void CUart0::setExtratimeOut(int x)
{
  this->extraTimeout=x;
}
/*********************************************************************/
void CUart0::enableTXInterrupt()
{
  IE1|=(UTXIE0);
}
/*********************************************************************/
void CUart0::disableTXInterrupt()
{
  IE1&=~(UTXIE0);
}
/*********************************************************************/
bool CUart0::isTXInterruptEnabled()
{
  if(IE1 & UTXIE0)
  {
    return true;
  }
  else
  {
    return false;
  }
}
/*********************************************************************/
void CUart0::configHandShackPorts()
{
  P2OUT&=~BT_CTS|BT_RTS;
  P2DIR&=~BT_RTS;       // Config as input
  P2DIR|=BT_CTS;        // Config as output
  P2IFG&=~BT_RTS;
  //P2IES&=~BT_RTS;  // Raise edge selection (Low to High transition)
  //P2IE|=BT_RTS;
}
/*********************************************************************/
bool CUart0::isBT_RTSUp()
{
  if(P2IN & BT_RTS)
    return true;
  return false;
}
/*********************************************************************/
/*void CUart0::setBT_CTS()
{
  P2OUT|=BT_CTS;
}*/
/*********************************************************************/
/*void CUart0::clearBT_CTS()
{
  P2OUT&=~BT_CTS;
}*/
/*********************************************************************/


