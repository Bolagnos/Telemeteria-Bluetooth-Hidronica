#include "CController.h"


/*********************************************************************/
/*Constructor*/
CPortDoorSw::CPortDoorSw(void)
{
  this->configPort();
}
/*********************************************************************/
/*Destructor*/
CPortDoorSw::~CPortDoorSw(void)
{}
/*********************************************************************/
void CPortDoorSw::configPort(void)
{
  P6DIR&=~portSw;   // Configuración como entrada para el switch de gabinete
}
/*********************************************************************/
bool CPortDoorSw::readSwitchPort(void)
{
  if(P6IN & portSw)
  {return true;}

  return false;
}
/*********************************************************************/
