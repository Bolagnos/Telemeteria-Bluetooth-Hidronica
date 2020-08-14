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
