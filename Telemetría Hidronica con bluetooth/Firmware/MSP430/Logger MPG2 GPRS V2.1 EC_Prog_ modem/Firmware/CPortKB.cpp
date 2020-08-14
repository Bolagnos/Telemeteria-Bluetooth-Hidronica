#include "CController.h"

/*********************************************************************/
/*Constructor*/

CPortKB::CPortKB(void)
{}
/*********************************************************************/
/*Destructor*/
CPortKB::~CPortKB(void)
{}
/*********************************************************************/
void CPortKB::configPortRows(void)
{
  //P5OUT&=~Row1;                  // Activa la salida  "0"
  P1OUT|=(Row1|Row2|Row3|Row4);     // Pone las salidas de control a "1"
  P1DIR|=Row1|Row2|Row3|Row4;   // Configuración como salida de control para teclado
}
/*********************************************************************/
void CPortKB::configPortCols(void)
{
  P1OUT|=(Col1|Col2|Col3);       // Pone las salidas de control a "1"
  P1DIR&=~(Col1|Col2|Col3);  // Configuración como entrada de control para teclado
  P1IFG=0x00;
}
/*********************************************************************/
void CPortKB::enableColInterrupt(char nPin)
{
  P1IE|=nPin;
}
/*********************************************************************/
void CPortKB::disableColInterrupt(char nPin)
{
  P1IE&=~nPin;
}
/*********************************************************************/
void CPortKB::setColEdgeHtoL(char nPin)
{
  P1IES|=nPin;
  P1IFG=0x00;
}
/*********************************************************************/
void CPortKB::setAllPortRows(void)
{
   P1OUT|=(Row1|Row2|Row3|Row4);   // Configuración como salida de control para teclado
}
/*********************************************************************/
