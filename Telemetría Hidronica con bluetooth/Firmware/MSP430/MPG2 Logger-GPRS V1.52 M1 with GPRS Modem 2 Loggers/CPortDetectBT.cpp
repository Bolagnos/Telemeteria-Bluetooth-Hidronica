#include "CController.h"

/******************************************************************************/
CPortDetectBT::CPortDetectBT()
{
  this->configPort();
}
/******************************************************************************/
CPortDetectBT::~CPortDetectBT()
{
}
/******************************************************************************/
void CPortDetectBT::configPort()
{
  P1DIR&=~portBTNProg;
  P1IES|=portBTNProg; // High to Low transition

  P3DIR|=portCtrlSWUART;
  P3OUT|=portCtrlSWUART; // Starts with Bluetooth disabled (GPRS UART port selected)

  P1IFG&=~portBTNProg;
  P1IE|=portBTNProg;
}
/******************************************************************************/
void CPortDetectBT::enablePort()
{
  P1IFG&=~portBTNProg;
  P1IE|=portBTNProg;
}
/******************************************************************************/
void CPortDetectBT::disablePort()
{
  P1IFG&=~portBTNProg;
  P1IE&=~portBTNProg;
}
/******************************************************************************/
bool CPortDetectBT::isBtnPressed()
{
  if(!(P1IN & portBTNProg))
  {
    return true;
  }
  return false;
}
/******************************************************************************/
void CPortDetectBT::enableUARTBluetooth()
{
  P3OUT&=~portCtrlSWUART; // Bluetooth UART port selected
}
/******************************************************************************/
void CPortDetectBT::disableUARTBluetooth()
{
  P3OUT|=portCtrlSWUART; // GPRS UART port selected
}
/******************************************************************************/
