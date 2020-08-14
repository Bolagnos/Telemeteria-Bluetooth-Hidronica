#include "stdlib.h"
#include "CController.h"
//#include "string"

/*********************************/

/* Constructors */
        CUART::CUART()
        {
          m_DMA=new CDMA();
          this->Buffer_Tx_Start=0;
          this->dataPtrRx=0;
          this->dataPtrTx=0;
          this->strRxLen=0;
          this->extraTimeCounter=0;
          this->commTries=0;

          this->statePort=0x00;    //Inicializa el estado del puerto de comunicaciones.

          this->UartError=0x00;
          P6DIR|=(ON_OFF_RS232|ENABLE_RS232);
          P6OUT|=ENABLE_RS232;
          P6OUT&=~ON_OFF_RS232;

          myWDT=new CWdTimer();
        }

/*********************************************************************/
/* Destructor	*/
	CUART::~CUART()
	{}
/*********************************************************************/
void CUART::configUart(CUART *Uartx,unsigned int portID,unsigned long bps, char sclk)
{
   Uartx->portID=portID;
   Uartx->bps=bps;
   Uartx->sclk=sclk;
   Uartx->Buffer_Tx_Start=0;
//   Uartx->Buffer_Rx[0]=0;
   Uartx->dataPtrRx=0;
   Uartx->dataPtrTx=0;
   Uartx->strRxLen=0;
   Uartx->extraTimeCounter=0;
   //Uartx->commTries=0;

   Uartx->statePort=0x00;    //Inicializa el estado del puerto de comunicaciones.

   Uartx->UartError=0x00;

}
/*********************************************************************/
char CUART::isOpen(CUART *Uartx)
{
 return Uartx->statePort & 0x01;
}
/*********************************************************************/
void CUART::closeUart()
{
  switch (this->portID)
  {
  case 0:
          UCTL0|=SWRST;           // Mantiene en estado de Reset la UART
          IE1&=~(URXIE0|UTXIE0); // Deshabilita interrupciones de la UART.
          ME1&=~(URXE0|UTXE0);   // Deshabilita los módulos de Rx y TX
          this->statePort&=~0x01;
          break;
  case 1:
          UCTL1|=SWRST;           // Mantiene en estado de Reset la UART
          IE2&=~(URXIE1|UTXIE1); // Deshabilita interrupciones de la UART.
          ME2&=~(URXE1|UTXE1);   // Deshabilita los módulos de Rx y TX
          this->statePort&=~0x01;
          break;

  }

}
/*********************************************************************/
char CUART::openUart(CUART *Uartx, bool spi)
{
  //this->closeUart();
   if(!this->isOpen(Uartx))  //Si esta cerrado
   {
      switch (this->getPortID(Uartx))
      {
      case 0:
             /*UCTL0|=SWRST;
             ME1&=~(URXE0|UTXE0|USPIE0);
             UCTL0|=CHAR;
             IFG1|=(UTXIFG0|URXIFG0);
             if(spi)
             {
                U0TCTL|=(STC|CKPH); // 3 wire , UCLK delayed by one half cycle
                UMCTL0=0x00;
                UCTL0|=(MM|SYNC);
                P3SEL&=~(BIT4|BIT5);
                P3SEL|=(BIT0|BIT1|BIT2|BIT3);
                ME1|=(USPIE0);    // Habilita los módulos de Rx y TX
             }
             else
             {
                UCTL0&=~(MM|SYNC);
                P3OUT&=~BIT5;
                P3OUT|=BIT4;
                P3SEL&=~(BIT0|BIT1|BIT2|BIT3);
                P3SEL|=(BIT4|BIT5);
                P3DIR&=~BIT5;          // Rx
                P3DIR|=BIT4;           // Tx
                ME1|=(URXE0|UTXE0);    // Habilita los módulos de Rx y TX
             }
             UCTL0&=~SWRST;         // Desactiva el estado de Reset la UART
             IE1|=(URXIE0);         // Habilita interrupciones Rx de la UART.
        */
             //this->commTries=0;
             //CClockModule::configSMCLK('O','0');
             if(spi)
             {
                this->commTries=0;
                this->configUart(this,0,400000,'S'); // Configures to 400 Kbps (400 KHz)
                this->statePort|=BIT4;
             }
             else
             {
                this->configUart(this,0,115200,'U');  // Configures to 9600 bps (9600 Hz) for bluetooth
                //this->configUart(this,0,9600,'A');  // Configures to 9600 bps (9600 Hz) for bluetooth
                this->statePort&=~BIT4;
             }
             this->setBaudRate(Uartx);
             UCTL0|=SWRST;
             ME1&=~(URXE0|UTXE0|USPIE0);
             UCTL0|=CHAR;
             IFG1&=~(UTXIFG0|URXIFG0);
             if(spi)
             {
                U0TCTL|=(STC|CKPH); // 3 wire , UCLK delayed by one half cycle
                UMCTL0=0x00;
                UCTL0|=(MM|SYNC);
                P3SEL&=~(BIT4|BIT5);
                P3SEL|=(BIT1|BIT2|BIT3); // SIMO0, SOMI0, UCLK0
                P3SEL&=~BIT0;            // STE0
                P3OUT&=~BIT0;            // STE0 = 0
                ME1|=(USPIE0);    // Habilita los módulos de Rx y TX
             }
             else
             {
                UCTL0&=~(MM|SYNC);
                P3OUT&=~BIT5;
                P3OUT|=BIT4;
                //P3SEL&=~(BIT0|BIT1|BIT2|BIT3); // STE0, SIMO0, SOMI0, UCLK0
                P3SEL&=~(BIT0|BIT1|BIT2); // STE0, SIMO0, SOMI0, UCLK0
                P3SEL|=(BIT4|BIT5);
                P3DIR&=~BIT5;          // Rx
                P3DIR|=BIT4;           // Tx
                ME1|=(URXE0|UTXE0);    // Habilita los módulos de Rx y TX
             }
             UCTL0&=~SWRST;         // Desactiva el estado de Reset la UART
             IE1|=(URXIE0);         // Habilita interrupciones Rx de la UART.
             break;
      case 1:
            // this->commTries=0;
             if(spi)
             {
                this->commTries=0;
                this->configUart(this,1,400000,'S'); // Conffigures to 400 Kbps (400 KHz)
                this->statePort|=BIT3;
             }
             else
             {
                this->configUart(this,1,9600,'A');  // Configures to 9600 bps (9600 Hz)
                this->statePort&=~BIT3;
             }
             this->setBaudRate(Uartx);
             UCTL1|=SWRST;
             ME2&=~(URXE1|UTXE1|USPIE1);
             UCTL1|=CHAR;
             IFG2&=~(UTXIFG1|URXIFG1);
             if(spi)
             {
                U1TCTL|=(STC|CKPH); // 3 wire , UCLK delayed by one half cycle
                UMCTL1=0x00;
                UCTL1|=(MM|SYNC);
                P3SEL&=~(BIT6|BIT7);
                P5SEL|=(BIT1|BIT2|BIT3);  // SIMO1, SOMI1, UCLK1
                P5SEL&=~BIT0;             // STE1
                P5OUT&=~BIT0;             // STE1 = 1
                ME2|=(USPIE1);    // Habilita los módulos de Rx y TX
             }
             else
             {
                UCTL1&=~(MM|SYNC);
                P3OUT&=~BIT7;
                P3OUT|=BIT6;
                P5SEL&=~(BIT0|BIT1|BIT2|BIT3); // STE1, SIMO1, SOMI1, UCLK1
                P3SEL|=(BIT6|BIT7);
                P3DIR&=~BIT7;          // Rx
                P3DIR|=BIT6;           // Tx
                ME2|=(URXE1|UTXE1);    // Habilita los módulos de Rx y TX
             }
             UCTL1&=~SWRST;         // Desactiva el estado de Reset la UART
             IE2|=(URXIE1);         // Habilita interrupciones Rx de la UART.
             break;
      }
      Uartx->statePort|=0x01;  //Establece el puerto como abierto (Ocupado).

      return Uartx->statePort;
   }
   else               //Si esta abierto.
   {return Uartx->statePort&0x01;}
}
/*********************************************************************/

unsigned int CUART::getPortID(CUART *Uartx)
{
   return Uartx->portID;
}
/*********************************************************************/
void CUART::setBaudRate(CUART *Uartx)
{
  long int clockFrec=32768;   // Frecuencia de reloj por default 32.768 KHz
  ldiv_t calc_rate;

  unsigned int dataBit=0;

  switch(Uartx->sclk)
  {
  //case 'U': clockFrec=7372800; P3SEL|=0x008; break;     //UCLK0 // Frecuencia de reloj externo de 7.3728 MHz
  case 'U': clockFrec=1843200; P3SEL|=BIT3; break;     //UCLK0 // Frecuencia de reloj externo de 1.8432 MHz
  case 'A': dataBit|=SSEL0; break;                      //ACLK
  case 'S': dataBit|=SSEL1; clockFrec=3200000; break;                      //SMCLK
  }

  calc_rate=ldiv(clockFrec,Uartx->bps);

  switch (this->portID)
  {
  case 0:
          U0TCTL=dataBit;
          U0BR0=calc_rate.quot & 0x0FF;
          U0BR1=(calc_rate.quot / 0x100)&0x0FF;
          break;
  case 1:
          U1TCTL=dataBit;
          U1BR0=calc_rate.quot & 0x0FF;
          U1BR1=(calc_rate.quot / 0x100)&0x0FF;
          break;
  }

  double myval=(clockFrec*1000/Uartx->bps);
  myval-=calc_rate.quot*1000;
  calcUARTModulationReg(myval/1000);

}
/*********************************************************************/
/*void CUart0::startTx(CUart0 *Uartx,char *BufferTx,int strLen)
{

   //Uartx->statePort|=0x02;
   Uartx->strTxLen=strLen;
   Uartx->Buffer_Tx_Start=(unsigned char*)BufferTx;
   Uartx->dataPtrTx=0;
   U0TXBUF=Uartx->Buffer_Tx_Start[Uartx->dataPtrTx];

}*/
/*********************************************************************/
/*int CUart0::getBuffRxLen(CUart0 *Uartx)
{
  return Uartx->strRxLen;
}*/
/*********************************************************************/
void CUART::clearRxBuffPointer()
{
  this->dataPtrRx=0;
}
/*********************************************************************/
void CUART::calcUARTModulationReg(double v_remain)
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
  switch (this->portID)
  {
  case 0:
          UMCTL0=reg; break;
         // UMCTL0=0x6B; break;
  case 1:
          UMCTL1=reg; break;
  }
}
/*********************************************************************/
void CUART::off_RS_232()
{
  P6OUT&=~ON_OFF_RS232;
  P6OUT|=ENABLE_RS232;
}
/*********************************************************************/
void CUART::on_RS_232()
{
  //int k;
  P6OUT|=ON_OFF_RS232;
  P6OUT&=~ENABLE_RS232;
//  for(k=0;k<12000;k++)
//  {}
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
/*char CUART::openUartAsSPI(CUART *Uartx)
{
   if(!isOpen(Uartx))  //Si esta cerrado
   {
      this->configUart(this,1,400000,'S');
      this->setBaudRate(Uartx);
      switch (getPortID(Uartx))
      {
      case 0:
             UMCTL0=0x00;
             UCTL0|=CHAR;
             UCTL0|=(MM|SYNC);
//             P3OUT&=~(BIT0|BIT1|BIT2|BIT3);
//             P3DIR&=~(BIT0|BIT1|BIT2|BIT3);
             P3SEL|=(BIT0|BIT1|BIT2|BIT3);
             IFG1|=(UTXIFG0|URXIFG0);
             ME1|=(USPIE0);    // Habilita los módulos de Rx y TX
             UCTL0&=~SWRST;         // Desactiva el estado de Reset la UART
             IE1|=(URXIE0);         // Habilita interrupciones Rx de la UART.

             //IE1|=(URXIE0|UTXIE0);  // Habilita interrupciones de la UART.
             break;
      case 1:
             UMCTL1=0x00;
             UCTL1|=CHAR;
             UCTL1|=(MM|SYNC);
//             P5OUT&=~(BIT0|BIT1|BIT2|BIT3);
//             P5DIR&=~(BIT0|BIT1|BIT2|BIT3);
             P5SEL|=(BIT0|BIT1|BIT2|BIT3);
             IFG2|=(UTXIFG1|URXIFG1);
             ME2|=(USPIE1);    // Habilita los módulos de Rx y TX
             UCTL1&=~SWRST;         // Desactiva el estado de Reset la UART
             IE2|=(URXIE1);         // Habilita interrupciones Rx de la UART.
             //IE2|=(URXIE1|UTXIE1);  // Habilita interrupciones de la UART.
             break;
      }
      Uartx->statePort|=0x01;  //Establece el puerto como abierto (Ocupado).

      return Uartx->statePort;
   }
   else               //Si esta abierto.
   {return Uartx->statePort&0x01;}
}*/
/*********************************************************************/
