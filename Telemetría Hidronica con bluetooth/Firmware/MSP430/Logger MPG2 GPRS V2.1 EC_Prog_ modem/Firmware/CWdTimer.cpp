#include "CController.h"

  CWdTimer::CWdTimer()
  {
    this->configWdt('R','R','T','A',0);
    this->stopWdt();
    this->eIntWdt();
  }
/*************************************************************************************************/
  CWdTimer::~CWdTimer()
  {}
/*************************************************************************************************/
  void CWdTimer::configWdt(char EdgeSel, char NMISel,char WTSel, char Clksrc, int DivSel )
  {
    int config=0;
    switch(EdgeSel)
    {
       case 'R':                      break;  //Raising
       case 'F': config|=WDTNMIES;    break;  //Falling
    }

    switch(NMISel)
    {
       case 'R':                      break;  //Reset
       case 'N': config|=WDTNMI;      break;  //NMI
    }

    switch(WTSel)
    {
       case 'W':                      break;  //Watchdog
       case 'T': config|=WDTTMSEL;    break;  //Timer
    }

    switch(Clksrc)
    {
       case 'S':                      break;  //SMCLK
       case 'A': config|=WDTSSEL;     break;  //ACLK
    }

    switch(DivSel)
    {
       case 0:                        break;  //32768
       case 1: config|=WDTIS0;        break;  //8192
       case 2: config|=WDTIS1;        break;  //512
       case 3: config|=WDTIS1+WDTIS0; break;  //64
    }

    WDTCTL=WDTPW|config|WDTHOLD;
  }
/*************************************************************************************************/
  void CWdTimer::stopWdt(void)
  {
    WDTCTL=WDTPW|((WDTCTL|WDTCNTCL|WDTHOLD)& 0x00FF);
  }
/*************************************************************************************************/
  void CWdTimer::startWdt(void)
  {
    WDTCTL=WDTPW|((WDTCTL|WDTCNTCL)&(~WDTHOLD)& 0x00FF);
  }
/*************************************************************************************************/
  void CWdTimer::restartWdt(void)
  {
    WDTCTL=WDTPW|((WDTCTL|WDTCNTCL)&(~WDTHOLD)& 0x00FF);
  }
/*************************************************************************************************/
  void CWdTimer::eIntWdt(void)
  {
    IE1|=WDTIE;
  }
/*************************************************************************************************/
  void CWdTimer::dIntWdt(void)
  {
    IE1&=~WDTIE;
  }
/*************************************************************************************************/
  bool CWdTimer::isHold(void)
  {
    if(WDTCTL & 0x0080)
    {
       return true;
    }
    return false;
  }
/*************************************************************************************************/
/*************************************************************************************************/
/*#pragma vector=WDT_VECTOR
  __interrupt void CWdTimer::WatchDogTimer1(void)
  {
    P5OUT^=0x01;
    stopWdt();

    //CUart::getRxBuffer(0);
    //CUart::getRXTail(0);
    //CUart::getRXTail(1);
  }
*/
/*************************************************************************************************/
/*************************************************************************************************/
/*************************************************************************************************/
/*************************************************************************************************/
/*************************************************************************************************/
