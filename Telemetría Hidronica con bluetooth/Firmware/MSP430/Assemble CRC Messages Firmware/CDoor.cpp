#include "CDevice.h"

/*********************************************************************/
CDoor::CDoor()
{
  this->portDoor = new CPortDoorSw();
  this->counterTime=0;
  this->doorSwitch=false;
}
/*********************************************************************/
CDoor::~CDoor()
{}
/*********************************************************************/
bool CDoor::evalIsDoorOpen()
{
  if(this->portDoor->readSwitchPort())
  {
    return false;
  }
  else
  {
    return true;
  }
}
/*********************************************************************/
void CDoor::setDoorOpenStateFlg()
{
  this->doorSwitch=true;
}
/*********************************************************************/
void CDoor::setDoorCloseStateFlg()
{
  this->doorSwitch=false;
}
/*********************************************************************/
bool CDoor::getDoorStateFlg()
{
  return this->doorSwitch;
}
/*********************************************************************/
void CDoor::incCounterTime()
{
  this->counterTime++;
}
/*********************************************************************/
void CDoor::clearCounterTime()
{
  this->counterTime=0;
}
/*********************************************************************/
bool CDoor::isCounterFull()
{
  if(this->counterTime==6)
  {
    this->counterTime=0;
    return true;
  }
  else
  {
    return false;
  }
}
/*********************************************************************/
