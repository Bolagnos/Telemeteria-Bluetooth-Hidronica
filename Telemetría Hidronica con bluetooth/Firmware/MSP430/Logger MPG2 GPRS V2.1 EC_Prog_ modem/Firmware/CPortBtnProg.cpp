#include "CController.h"

/******************************************************************************/
CPortBtnProg::CPortBtnProg()
{
  this->configPort();
}
/******************************************************************************/
CPortBtnProg::~CPortBtnProg()
{
}
/******************************************************************************/
void CPortBtnProg::configPort()
{
  P1DIR&=~portBTNProg;
  P1IES|=portBTNProg; // High to Low transition
  P1IFG&=~portBTNProg;
  P1IE|=portBTNProg;
}
/******************************************************************************/
void CPortBtnProg::enablePort()
{
  P1IFG&=~portBTNProg;
  P1IE|=portBTNProg;
}
/******************************************************************************/
void CPortBtnProg::disablePort()
{
  P1IFG&=~portBTNProg;
  P1IE&=~portBTNProg;
}
/******************************************************************************/
bool CPortBtnProg::isBtnPressed()
{
  if(!(P1IN & portBTNProg))
  {
    return true;
  }
  return false;
}
/******************************************************************************/
