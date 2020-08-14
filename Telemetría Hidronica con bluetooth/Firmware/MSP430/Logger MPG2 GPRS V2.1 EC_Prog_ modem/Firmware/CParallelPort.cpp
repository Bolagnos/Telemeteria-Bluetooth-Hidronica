#include "CController.h"

#define LCD_RS  BIT0  // RS                 P3.0
#define LCD_RW  BIT1  // RW                 P3.1
//#define LCD_E   BIT2  // E                  P3.2
#define LCD_BL  BIT0  // Back Light Control P5.0

/*********************************************************************/
/*Constructor*/

CParallelPort::CParallelPort(void)
{}
/*********************************************************************/
/*Destructor*/
CParallelPort::~CParallelPort(void)
{}
/*********************************************************************/
void CParallelPort::configPortDataLCD(void)
{
  P2OUT=0x00;   // Pone las salida de datos a "0"
  P2DIR|=0xFF;  // Configuración como salida de datos para LCD
}
/*********************************************************************/

void CParallelPort::configPortCtrlLCD(void)
{
  P3OUT&=~(LCD_RS|LCD_RW); //|LCD_E); // Pone las salidas de control a "0"
  P3DIR|=LCD_RS|LCD_RW; //|LCD_E;  // Configuración como salida de control para LCD

  P5OUT&=~(LCD_BL); // Pone las salidas de control del BL a "0"
  P5DIR|=LCD_BL;  // Configuración como salida de control para LCD
}
/*********************************************************************/
/*void CParallelPort::sendData(char data)
{
  P4OUT=data;
}*/
/*********************************************************************/
/*void CParallelPort::sendControl(char data)
{
  P5OUT=data;
}*/
/*********************************************************************/
void CParallelPort::commandInstr(char command)
{
  P3OUT&=~(LCD_RS|LCD_RW); //|LCD_E);
  //_NOP();
  //_NOP();
  //P3OUT|=LCD_E;
  //_NOP();
  //_NOP();
  P2OUT=command;
  //_NOP();
  //_NOP();
  //P3OUT&=~LCD_E;
  P3OUT|=LCD_RS;
  for(int x=0;x<100;x++)
  {}
}
/*********************************************************************/
void CParallelPort::writeInstr(char command)
{
  P3OUT&=~(LCD_RW); //|LCD_E);
  P3OUT|=LCD_RS;
  //_NOP();
  //_NOP();
  //P3OUT|=LCD_E;
  //_NOP();
  //_NOP();
  P2OUT=command;
  //_NOP();
  //_NOP();
  //P3OUT&=~LCD_E;
  P3OUT&=~LCD_RS;
//  for(int x=0;x<80;x++)
//  {}

}
/*********************************************************************/
/*********************************************************************/
void CParallelPort::setLCD_BL()
{
  P5OUT|=LCD_BL;
}
/*********************************************************************/
void CParallelPort::clearLCD_BL()
{
  P5OUT&=~LCD_BL;
}
/*********************************************************************/
/*#define clear_display   0x01
#define return_home     0x02
#define entry_mode      0x04
#define display_on_off  0x08
#define cursor_display  0x10
#define function_set    0x20
#define cg_ram_addr     0x40
#define dd_ram_addr     0x80

void sendMessage(char addr1,char length1,char* Message1,char addr2,char length2,char* Message2)
{
  int x;
  commandInstr(addr1|dd_ram_addr);
  for(x=0;x<length1;x++)
  {
    writeInstr(Message1[x]);
  }

  commandInstr(addr2|dd_ram_addr);
  for(x=0;x<length2;x++)
  {
    writeInstr(Message2[x]);
  }
}*/
/*********************************************************************/
