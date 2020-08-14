#include "CDevice.h"

#define clear_display   0x01
#define return_home     0x02
#define entry_mode      0x04
#define display_on_off  0x08
#define cursor_display  0x10
#define function_set    0x20
#define cg_ram_addr     0x40
#define dd_ram_addr     0x80

/*******************************************************************************************/
CLcd::CLcd()
{
  PortHandler=new CParallelPort();
  this->configLCDPort();
  this->configLCD();
  this->PortHandler->commandInstr(clear_display);
  this->BLCounter=0x00;
}
/*******************************************************************************************/
CLcd::~CLcd()
{}
/*******************************************************************************************/
void CLcd::configLCDPort(void)
{
  this->PortHandler->configPortDataLCD();
  this->PortHandler->configPortCtrlLCD();
}
/*******************************************************************************************/

void CLcd::sendMessage(char addr1,char length1,char* Message1,char addr2,char length2,char* Message2, bool LCDClear)
{
  int x;

  if (LCDClear)
  {
    this->PortHandler->commandInstr(clear_display);
    for(x=0;x<200;x++)
    {
    }
  }
//  _NOP();

  this->PortHandler->commandInstr(addr1|dd_ram_addr);
  for(x=0;x<length1;x++)
  {
    this->PortHandler->writeInstr(Message1[x]);
  }

  this->PortHandler->commandInstr(addr2|dd_ram_addr);
  for(x=0;x<length2;x++)
  {
    this->PortHandler->writeInstr(Message2[x]);
  }


/*  for(x=0;x<60;x++)
  {
  this->PortHandler->commandInstr(0x08|cursor_display);
  }
*/

}
/*********************************************************************/
void CLcd::configLCD(void)
{
//  this->PortHandler->commandInstr(0x18|function_set);
  this->PortHandler->commandInstr(0x18|function_set);
  this->PortHandler->commandInstr(0x04|display_on_off);
  this->PortHandler->commandInstr(0x02|entry_mode);

  //this->PortHandler->commandInstr(clear_display);
  this->PortHandler->commandInstr(return_home);
  this->PortHandler->commandInstr(0x00|cursor_display);


}
/*******************************************************************************************/
/*#define clear_display   0x01
#define return_home     0x02
#define entry_mode      0x04
#define display_on_off  0x08
#define cursor_display  0x10
#define function_set    0x20
#define cg_ram_addr     0x40
#define dd_ram_addr     0x80


*/

#define cursorOn_Off  0x02
#define cursorBlink    0x01

void CLcd::configLCDEdit(void)
{
//  this->PortHandler->commandInstr(0x18|function_set);
//  this->PortHandler->commandInstr(0x18|function_set);
  this->PortHandler->commandInstr(cursorBlink|cursorOn_Off|0x04|display_on_off);
  //this->PortHandler->commandInstr(entry_mode);

  //this->PortHandler->commandInstr(clear_display);
  //this->PortHandler->commandInstr(return_home);
  this->PortHandler->commandInstr(0x00|cursor_display);


}

void CLcd::exitLCDEdit(void)
{
  this->PortHandler->commandInstr(0x04|display_on_off);
  this->PortHandler->commandInstr(0x00|cursor_display);
}


void CLcd::setAddr(char cAddr)
{
    this->PortHandler->commandInstr(cAddr|dd_ram_addr);
}

void CLcd::setCaracter(char cData)
{
    this->PortHandler->writeInstr(cData);
}

void CLcd::shiftLCDLeft()
{
  this->PortHandler->commandInstr(0x00|cursor_display);
}

void CLcd::shiftLCDRight()
{
  this->PortHandler->commandInstr(0x04|cursor_display);
}

/*********************************************************************/
void CLcd::onBackLight()
{
  this->PortHandler->setLCD_BL();
}
/*********************************************************************/
void CLcd::offBackLight()
{
  this->PortHandler->clearLCD_BL();
}
/*********************************************************************/
void CLcd::incBLCounter()
{
  BLCounter++;
}
/*********************************************************************/
char CLcd::getBLCounter()
{
  return this->BLCounter;
}
/*********************************************************************/
void CLcd::clearBLCounter()
{
  BLCounter=0x00;;
}
/*********************************************************************/

