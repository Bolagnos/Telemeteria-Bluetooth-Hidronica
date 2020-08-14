#include "CController.h"

/*********************************************************************/
/*Constructor*/
CPortRGBLED::CPortRGBLED(void)
{
  this->configPort();
  this->offAllLED();
}
/*********************************************************************/
/*Destructor*/
CPortRGBLED::~CPortRGBLED(void)
{}
/*********************************************************************/
void CPortRGBLED::configPort(void)
{
  P5DIR|= (RedLED | GreenLED | BlueLED)|onSystemLED;   // Configuración como salida para indicador RGB LED
  P5OUT&=~((RedLED | GreenLED | BlueLED)|onSystemLED);     // Apaga todos los led indicadores RGB
}
/*********************************************************************/
void CPortRGBLED::offAllLED()
{
  P5OUT&= ~(RedLED | GreenLED | BlueLED);     // Apaga todos los led indicadores RGB
}
/*********************************************************************/
void CPortRGBLED::onRedLED()
{
  P5OUT&=~(GreenLED | BlueLED);
  P5OUT|=(RedLED);
}
/*********************************************************************/
void CPortRGBLED::onGreenLED()
{
  P5OUT&=~(RedLED | BlueLED);
  P5OUT|=(GreenLED);
}
/*********************************************************************/
void CPortRGBLED::onBlueLED()
{
  P5OUT&=~(RedLED | GreenLED);
  P5OUT|=(BlueLED);
}
/*********************************************************************/
void CPortRGBLED::onGreen_BlueLED()
{
  P5OUT&=~(RedLED);
  P5OUT|=(GreenLED | BlueLED);
}
/*********************************************************************/
void CPortRGBLED::onRed_BlueLED()
{
  P5OUT&=~(GreenLED);
  P5OUT|=(RedLED | BlueLED);
}
/*********************************************************************/
void CPortRGBLED::onRed_GreenLED()
{
  P5OUT&=~(BlueLED);
  P5OUT|=(RedLED | GreenLED);
}
/*********************************************************************/
void CPortRGBLED::onAllLED()
{
  P5OUT|=(RedLED | GreenLED | BlueLED);
}
/*********************************************************************/
void CPortRGBLED::changeStateOnSystemLED()
{
  P5OUT^=onSystemLED;
}
/*********************************************************************/
