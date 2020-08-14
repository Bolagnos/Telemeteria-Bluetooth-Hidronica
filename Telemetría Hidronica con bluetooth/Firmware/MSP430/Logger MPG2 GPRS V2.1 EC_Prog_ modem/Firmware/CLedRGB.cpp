#include "CDevice.h"

/*********************************************************************/
CLedRGB::CLedRGB()
{
  this->ledRGBPort = new CPortRGBLED();
  this->counterTime=0;
}
/*********************************************************************/
CLedRGB::~CLedRGB()
{}
/*********************************************************************/
void CLedRGB::offAll()
{
  this->ledRGBPort->offAllLED();
}
/*********************************************************************/
void CLedRGB::onOnlyRed()
{
  this->ledRGBPort->onRedLED();
}
/*********************************************************************/
void CLedRGB::onOnlyGreen()
{
  this->ledRGBPort->onGreenLED();
}
/*********************************************************************/
void CLedRGB::onOnlyBlue()
{
  this->ledRGBPort->onBlueLED();
}
/*********************************************************************/
void CLedRGB::onMagenta()
{
  this->ledRGBPort->onRed_BlueLED();
}
/*********************************************************************/
void CLedRGB::onYellow()
{
  this->ledRGBPort->onRed_GreenLED();
}
/*********************************************************************/
void CLedRGB::onCyan()
{
  this->ledRGBPort->onGreen_BlueLED();
}
/*********************************************************************/
void CLedRGB::onWhite()
{
  this->ledRGBPort->onAllLED();
}
/*********************************************************************/
bool CLedRGB::isTimeToLighting()
{
  this->counterTime++;
  if(this->counterTime==6) // 6 Seconds
  {
    this->counterTime=0;
    return true;
  }
  return false;
}
/*********************************************************************/
void CLedRGB::changeStateOnLED()
{
  this->ledRGBPort->changeStateOnSystemLED();
}
/*********************************************************************/
