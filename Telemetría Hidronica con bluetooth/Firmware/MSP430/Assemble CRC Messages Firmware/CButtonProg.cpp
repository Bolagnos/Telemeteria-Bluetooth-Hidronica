#include "CDevice.h"

/****************************************************************************/
CButtonProg::CButtonProg()
{
  this->configBtnProg();
  this->clearProgFlag();
  //this->counter=0;
}
/****************************************************************************/
CButtonProg::~CButtonProg()
{
}
/****************************************************************************/
void CButtonProg::configBtnProg()
{
  this->btnProg = new CPortDetectBT();
}
/****************************************************************************/
void CButtonProg::enableButton()
{
  this->btnProg->enablePort();
}
/****************************************************************************/
void CButtonProg::disableButton()
{
  this->btnProg->disablePort();
}
/****************************************************************************/
bool CButtonProg::isButtonPressed()
{
  return this->btnProg->isBtnPressed();
}
/****************************************************************************/
void CButtonProg::setProgFlag()
{
  this->btnProg->enableUARTBluetooth();
  this->btnFlag|=BIT0;
}
/****************************************************************************/
void CButtonProg::clearProgFlag()
{
  this->btnProg->disableUARTBluetooth();
  this->btnFlag&=~BIT0;
}
/****************************************************************************/
bool CButtonProg::isProgFlagActivated()
{
  if(this->btnFlag & BIT0)
  {
    return true;
  }
  return false;
}
/****************************************************************************/
/*void CButtonProg::clearCounterSeconds()
{
  this->counter=0;
}*/
/****************************************************************************/
/*void CButtonProg::incCounterSeconds()
{
  this->counter++;
}*/
/****************************************************************************/
/*bool CButtonProg::isCounterAtLimit()
{
  this->incCounterSeconds();
  if(this->counter >= 120) // Cumplió dos minutos
  {
    this->clearCounterSeconds();
    return true;
  }
  return false;
}*/
/****************************************************************************/
