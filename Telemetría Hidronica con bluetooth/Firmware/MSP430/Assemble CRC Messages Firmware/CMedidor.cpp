#include "CMedidor.h"

const unsigned long int initWRAddrMed [] ={0x060000,0x061000,0x062000,0x063000};
/****************************************************************************************************/
CMedidor::CMedidor(char index)
{
//  unsigned int startAddress;

/*  switch(index)
  {
  case 1:   startAddress=2206; break;
  case 2:   startAddress=2211; break;
  case 3:   startAddress=2216; break;
  case 0:
  default:  startAddress=2201; break;
  }*/
//////  this->setStartAddressMB(startAddress);  // Establece la dirección de inicio en 2201
  this->setPointerWrite(initWRAddrMed[index]);  // Establece la dirección de inicio del puntero de escritura de la memoria EEPROM
  this->setIDVariable(0x01);                    // Establece reporete de gasto y volumen
}
/****************************************************************************************************/
CMedidor::~CMedidor()
{

}
/****************************************************************************************************/
/*void CMedidor::setStartAddressMB(unsigned int startAddr)
{
  this->startAddrMODBUS=startAddr-1;
}*/
/****************************************************************************************************/
/*unsigned int* CMedidor::getStartAddressMB()
{
  return &this->startAddrMODBUS;
}*/
/****************************************************************************************************/
void CMedidor::setIDVariable(char IDRegistro)
{
  this->idRegistro=IDRegistro;
}
/****************************************************************************************************/
char* CMedidor::getIDVariable()
{
  return &this->idRegistro;
}
/****************************************************************************************************/
/*void CMedidor::setRegFlowrate(unsigned int* FlowRate)
{
  this->flowRateFT=*FlowRate;
}*/
/****************************************************************************************************/
/*unsigned int* CMedidor::getRegFlowrate()
{
  return &this->flowRateFT;
}*/
/****************************************************************************************************/
/*void CMedidor::setRegTotalizer(unsigned long* Totalizer)
{
  this->totalizerFT=*Totalizer;
}*/
/****************************************************************************************************/
/*unsigned long* CMedidor::getRegTotalizer()
{
  return &this->totalizerFT;
}*/
/****************************************************************************************************/
/*void CMedidor::setRegLevel(unsigned int* Level)
{
  this->levelFT=*Level;
}*/
/****************************************************************************************************/
/*unsigned int* CMedidor::getRegLevel()
{
  return &this->levelFT;
}*/
/****************************************************************************************************/
/*void CMedidor::setRegAlarm(char* Alarm)
{
  this->alarm=*Alarm;
}*/
/****************************************************************************************************/
/*char* CMedidor::getRegAlarm()
{
  return (char*)&this->alarm;
}*/
/****************************************************************************************************/
void CMedidor::setPointerWrite(unsigned long int pWrMem)
{
  this->p_memLoggerWrite=pWrMem;
}
/****************************************************************************************************/
unsigned long int* CMedidor::getPointerWrite()
{
  return &this->p_memLoggerWrite;
}
/****************************************************************************************************/
void CMedidor::addPointerWrite(int NBytes)
{
  this->p_memLoggerWrite+=NBytes;
}
/****************************************************************************************************/
const unsigned long int* CMedidor::getEEPROMstartAddr(int index)
{
  return &initWRAddrMed[index];
}
/****************************************************************************************************/
void CMedidor::restartWRpointer(int index)
{
  this->setPointerWrite(initWRAddrMed[index]); // establece la dirección de inicio del puntero de escritura de la memoria EEPROM
}
/****************************************************************************************************/
void CMedidor::restartWRpointerOffset(int index, int offset)
{
  this->setPointerWrite(initWRAddrMed[index]+offset); // establece la dirección de inicio del puntero de escritura de la memoria EEPROM
}
/****************************************************************************************************/
/*void CMedidor::setRegDolevas(char* NDolevas)
{
  this->nDolevas=*NDolevas;
}*/
/****************************************************************************************************/
/*char* CMedidor::getRegDolevas()
{
  return (char*)&this->nDolevas;
}*/
/****************************************************************************************************/

