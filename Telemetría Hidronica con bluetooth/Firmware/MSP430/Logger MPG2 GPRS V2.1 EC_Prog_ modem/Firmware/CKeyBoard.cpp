#include "CDevice.h"

CKeyBoard::CKeyBoard()
{
  KBFlags=0x00;
  //this->tempA=new CTimerA();
  this->KBPHandler=new CPortKB();
  this->configKB();
  this->enableKBoard();
}
/*******************************************************************************************/
CKeyBoard::~CKeyBoard()
{

}
/*******************************************************************************************/
void CKeyBoard::configKB()
{
  this->KBPHandler->configPortRows();
  this->KBPHandler->configPortCols();
  this->KBPHandler->setColEdgeHtoL(Col1|Col2|Col3);
  //this->tempA->disableTAIE();
  this->KBPHandler->enableColInterrupt(Col1|Col2|Col3); // habilita el signo # en el teclado como entrada a menu (interrupcion de puerto)
}
/*******************************************************************************************/
void CKeyBoard::enableKBoard()
{
   //this->configKB();
   //this->KBPHandler->setColEdgeHtoL(Col1|Col2|Col3);
   //this->tempA->disableTAIE();
   this->KBFlags|=0x01;
   //this->KBPHandler->enableColInterrupt(Col1|Col2|Col3); // habilita el signo # en el teclado como entrada a menu (interrupcion de puerto)
}
/*******************************************************************************************/
void CKeyBoard::disableKBoard()
{
   this->KBFlags&=~0x01;
   //this->KBPHandler->disableColInterrupt(Col1|Col2|Col3); // deshabilita el signo # en el teclado como entrada a menu (interrupcion de puerto)
   //this->KBPHandler->setAllPortRows();
   //this->tempA->enableTAIE();
}
/*******************************************************************************************/
char CKeyBoard::getKeyPushed()
{
  char selRow, selCol;


  selRow=(P1OUT & 0x78);
  selRow/=Row1;
  selRow^=0x0F;

  selCol=(P1IFG & 0x07);
  selCol/=Col1;

  switch(selRow)
  {
  case 1:  // Teclas 1,2,3
          switch (selCol)
          {
          case 1: selRow=0x31; break;
          case 2: selRow=0x32; break;
          case 4: selRow=0x33; break;
          }
          break;
  case 2: // Teclas 4,5,6
          switch (selCol)
          {
          case 1: selRow=0x34; break;
          case 2: selRow=0x35; break;
          case 4: selRow=0x36; break;
          }
          break;
  case 4: // Teclas 7,8,9
          switch (selCol)
          {
          case 1: selRow=0x37; break;
          case 2: selRow=0x38; break;
          case 4: selRow=0x39; break;
          }
          break;
  case 8: // Teclas *,0,#
          switch (selCol)
          {
          case 1: selRow=0x2A; break;
          case 2: selRow=0x30; break;
          case 4: selRow=0x23; break;
          }
          break;
  default:
          break;

  }
  return selRow;
}
/*******************************************************************************************/
/*void CKeyBoard::enableTempA()
{
   this->tempA->enableTAIE();
}*/
/*******************************************************************************************/
/*void CKeyBoard::disableTempA()
{
   this->tempA->disableTAIE();
}*/
/*******************************************************************************************/


