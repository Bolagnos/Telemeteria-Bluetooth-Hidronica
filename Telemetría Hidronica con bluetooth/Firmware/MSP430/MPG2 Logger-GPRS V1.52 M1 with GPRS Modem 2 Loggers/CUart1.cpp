#include "stdlib.h"
#include "CController.h"
#include "string"

/*********************************/

/* Constructors */
        CUart1::CUart1()
        {
          this->m_DMA->setDMASourceAddr_CH1((char*)this->Buffer_Tx);
          this->extraTimeout=2;
          this->tries=12;
          this->Buffer_Rx[0]=0;
          this->closeUart();
          this->portID=0x01;
        }

/*********************************************************************/
/* Destructor	*/
	CUart1::~CUart1()
	{}
/*********************************************************************/
void CUart1::startTx(char *BufferTx, int strLen)
{

   this->on_RS_232();
   this->statePort|=0x02;

   /*this->m_DMA->setDMASourceAddr_CH1((char*)&BufferTx[1]);
   this->m_DMA->setTx1_Size(strLen-1);
   this->m_DMA->setDMA_CH1_EN();*/

   this->m_DMA->setDMASourceAddr_CH2((char*)&BufferTx[1]);
   this->m_DMA->setTx2_Size(strLen-1);
   this->m_DMA->setDMA_CH2_EN();

   IFG2&=~(UTXIFG1|URXIFG1);
   U1TXBUF=BufferTx[0];
}
/*********************************************************************/
void CUart1::startTx(char *BufferTx, int strLenTx, char *BufferRx, int strLenRx)
{
   this->m_DMA->setDMASourceAddr_CH2((char*)&BufferTx[1]);
   this->m_DMA->setTx2_Size(strLenTx-1);
   this->m_DMA->setDMA_CH2_EN();
//   this->m_DMA->setDMA_CH2_REQ();

   IFG2&=~(UTXIFG1|URXIFG1);
   U1TXBUF=BufferTx[0];
}
/*********************************************************************/
void CUart1::startTxRx(char *BufferTx, int strLenTx, char *BufferRx, int strLenRx)
{
   this->m_DMA->setDMADestinyAddr_CH1((char*)&BufferRx[0]);
   this->m_DMA->setRx1_Size(strLenRx);
   this->m_DMA->setDMA_CH1_EN();
   //this->m_DMA->setDMA_CH1_REQ();

   this->m_DMA->setDMASourceAddr_CH2((char*)&BufferTx[0]);
   this->m_DMA->setTx2_Size(strLenTx-1);
   this->m_DMA->setDMA_CH2_EN();
   //this->m_DMA->setDMA_CH2_REQ();

   IFG2&=~(UTXIFG1|URXIFG1);
   U1TXBUF=BufferTx[0];
}
/*********************************************************************/
int CUart1::getBuffRxLen()
{
  return this->strRxLen;
}
/*********************************************************************/
CUart1* CUart1::getUART()
{
  return this;
}
/*********************************************************************/
void CUart1::configDMAtoReadMem()
{
  //this->closeUart();
  this->m_DMA->configDMAfor_TXRX_UART_SDMem();
  //this->openUart(this,true);
  //IFG2&=~(UTXIFG1|URXIFG1);
  ME2|=(USPIE1);    // Habilita los módulos de Rx y TX
}
/*********************************************************************/
void CUart1::configDMAtoWriteMem()
{
  //this->closeUart();
  this->m_DMA->configDMAfor_TX_UART_SDMem();
  //this->openUart(this,true);
  //ME2|=(USPIE1);    // Habilita los módulos de Rx y TX
}
/*********************************************************************/
void CUart1::configDMAforFlowmeter()
{
  //this->closeUart();
  this->m_DMA->configDMAfor_TXUARTs();
  //this->openUart(this,false);
}
/*********************************************************************/
