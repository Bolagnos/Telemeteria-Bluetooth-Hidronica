#include "CController.h"


  CTimerA::CTimerA()
  {
    this->configTA('A',0x01,'U');
    this->clearTACounter();
//    this->enableTAIE();
  }
/*************************************************************************************************/
  CTimerA::~CTimerA()
  {}
/*************************************************************************************************/
/******************************************************************************/
int CTimerA::configTA(char srcClk1,char divisor,char modectrl)
{
   unsigned int my_register=0x0000;

   //if((srcClk1=='T' || srcClk1=='A' || srcClk1=='S' || srcClk1=='I') && (modectrl=='S' || modectrl=='U' || modectrl=='C' || modectrl=='D') && (divisor>=0x01 && divisor<=0x04))
   //{

      switch(srcClk1)
      {
      case 'T': my_register|=TASSEL_0;           break;
      case 'A': my_register|=TASSEL_1;           break;
      case 'S': my_register|=TASSEL_2;           break;
      case 'I': my_register|=TASSEL_3;           break;
      default: break;
      }

      switch(divisor)
      {
      case 0x01: my_register|=ID_0;           break;
      case 0x02: my_register|=ID_1;           break;
      case 0x03: my_register|=ID_2;           break;
      case 0x04: my_register|=ID_3;           break;
      default: break;
      }

      switch(modectrl)
      {
      case 'S': my_register|=MC_0;           break;
      case 'U': my_register|=MC_1;           break;
      case 'C': my_register|=MC_2;           break;
      case 'D': my_register|=MC_3;           break;
      default: break;
      }


      this->clearTACounter();
      TACCR0=0x8000;      // Equivalencia a 0.25 seg (32.768 KHz *0.25) Time out por bit.
      //TACCR1=0x8000;      // Equivalencia a 1.00 seg (32.768 KHz *1.00) Time out de cadena.
      TACTL=my_register;

      //this->enableTAIE();


      return 1;
   //}

   //return 0;    // Devuelve error de paso de parametros.
}
/******************************************************************************/
void CTimerA::clearTACounter(void)
{
  TACTL|=TACLR;
}
/******************************************************************************/
unsigned int CTimerA::getTACNT(void)
{
  return (unsigned int)TAR;
}
/******************************************************************************/
void CTimerA::enableTAIE()
{
  TACTL|=TAIE;
}
/******************************************************************************/
void CTimerA::disableTAIE()
{
   TACTL&=~TAIE;
}
/******************************************************************************/
void CTimerA::clearTAIFG()
{
 TACTL&=~TAIFG;
}
/******************************************************************************/





