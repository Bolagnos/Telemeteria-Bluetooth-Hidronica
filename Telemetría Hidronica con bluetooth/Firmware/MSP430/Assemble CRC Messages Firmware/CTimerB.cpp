#include "CController.h"
/*************************************************************************************************/
  CTimerB::CTimerB()
  {
    this->configTB('A',0x01,'U');
    this->clearTBCounter();
    this->ConfigTB0asCapture();
    //this->enableTBIE();
  }
/*************************************************************************************************/
  CTimerB::~CTimerB()
  {}
/******************************************************************************/
int CTimerB::configTB(char srcClk1,char divisor,char modectrl)
{
   unsigned int my_register=0x0000;

   //if((srcClk1=='T' || srcClk1=='A' || srcClk1=='S' || srcClk1=='I') && (modectrl=='S' || modectrl=='U' || modectrl=='C' || modectrl=='D') && (divisor>=0x01 && divisor<=0x04))
   //{

      switch(srcClk1)
      {
      case 'T': my_register|=TBSSEL_0;           break;
      case 'A': my_register|=TBSSEL_1;           break;
      case 'S': my_register|=TBSSEL_2;           break;
      case 'I': my_register|=TBSSEL_3;           break;
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

      this->clearTBCounter();
      TBCCR0=0x8000;      // Equivalencia a 0.25 seg (32.768 KHz *0.25) Time out por bit.
      //TBCCR1=0x8000;      // Equivalencia a 1.00 seg (32.768 KHz *1.00) Time out de cadena.
      TBCTL=my_register;
      //this->enableTBIE();




      return 1;
   //}

   //return 0;    // Devuelve error de paso de parametros.
}
/******************************************************************************/
void CTimerB::clearTBCounter(void)
{
  TBCTL|=TBCLR;
}
/******************************************************************************/
unsigned int CTimerB::getTBCNT(void)
{
  return (unsigned int)TBR;
}
/******************************************************************************/
void CTimerB::enableTBIE()
{
  TBCTL|=TBIE;
}
/******************************************************************************/
void CTimerB::disableTBIE()
{
   TBCTL&=~TBIE;
}
/******************************************************************************/
void CTimerB::clearTBIFG()
{
 TBCTL&=~TBIFG;
}
/******************************************************************************/
void CTimerB::ConfigTB0asCapture()
{
  TBCCTL0=CM_1|CCIS_1|CAP;
}
/******************************************************************************/
void CTimerB::EnableTBCCTL0_IE()
{
  TBCCTL0|=CCIE;
}
/******************************************************************************/
void CTimerB::DisableTBCCTL0_IE()
{
  TBCCTL0&=~CCIE;
}
/******************************************************************************/




