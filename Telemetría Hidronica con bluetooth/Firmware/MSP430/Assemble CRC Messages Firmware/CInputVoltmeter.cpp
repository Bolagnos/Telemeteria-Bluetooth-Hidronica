#include "CDevice.h"

/****************************************************************************************************/
CInputVoltmeter::CInputVoltmeter()
{
  this->m_ADC = new CADC12();
  this->m_ADC->NPressDevices=NAnalogDevices;
  this->m_ADC->RPointer=0;
  this->m_ADC->flagLogger=0;
  this->addVoltage[0]=0;
  this->counterReg=0;
  this->Vpot=0;
  this->flgRequest=false;
  this->m_ADC->configADC12(NAnalogDevices);
}
/****************************************************************************************************/
CInputVoltmeter::~CInputVoltmeter()
{}
/****************************************************************************************************/
void CInputVoltmeter::startCollector()
{
  this->m_ADC->flagLogger=0;
  this->m_ADC->startADC12();
}
/****************************************************************************************************/
void CInputVoltmeter::stopCollector()
{
  this->m_ADC->stopADC12();
}
/****************************************************************************************************/
void CInputVoltmeter::getSample()
{
  //unsigned int *PBufferRec;
  unsigned short *MemoADCChannel;

  if (this->m_ADC->RPointer==nsamplesADC)
  {
      this->stopCollector();
      this->m_ADC->flagLogger=0x01;      // Habilita bandera de fin de recolección
      this->m_ADC->RPointer=0;       // Inicializa el puntero del buffer de Rx si hay un overflow
      this->Vpot=(this->addVoltage[0]/nsamplesADC) * (2.52/0x0FFF);
      this->addVoltage[0]=0;
  }
  else
  {
     //PBufferRec=this->m_ADC->myBufferDataM0;
     MemoADCChannel=(unsigned short *)&ADC12MEM0;

     for(int x=0; x<this->m_ADC->NPressDevices; x++)
     {
       //PBufferRec[this->m_ADC->RPointer]=(*MemoADCChannel); //-offset;
       //this->addVoltage[x]+=PBufferRec[this->m_ADC->RPointer];
       this->addVoltage[x]+=(unsigned int)(*MemoADCChannel);
       MemoADCChannel++;
       //PBufferRec+=nsamplesADC;
     }
     this->m_ADC->RPointer++;
  }

  this->m_ADC->clearADCflags();

}
/****************************************************************************************************/
char CInputVoltmeter::isCollectedFlag()
{
  return this->m_ADC->flagLogger;
}
/****************************************************************************************************/
double CInputVoltmeter::getVpot()
{
  return this->Vpot;
}
/****************************************************************************************************/
double CInputVoltmeter::getVoltageBatt()
{
  this->startCollector();
  while(!this->isCollectedFlag())
  {}
  return this->getVpot();
}
/****************************************************************************************************/
void CInputVoltmeter::setFlagReq()
{
  this->flgRequest=true;
}
/****************************************************************************************************/
void CInputVoltmeter::clearFlagReq()
{
  this->flgRequest=false;
}
/****************************************************************************************************/
bool CInputVoltmeter::isFlagReq()
{
  return this->flgRequest;
}
/****************************************************************************************************/
void CInputVoltmeter::incCounterReg()
{
  this->counterReg++;
}
/****************************************************************************************************/
void CInputVoltmeter::clearCounterReg()
{
  this->counterReg=0;
}
/****************************************************************************************************/
bool CInputVoltmeter::incAndCompCounter()
{
  if(!this->isFlagReq())
  {
    this->incCounterReg();
    if(this->counterReg>=7200)  // Enables flgRequest each 21600 seconds (6 hours); 7200 = 2hrs
    {
      this->clearCounterReg();
      this->setFlagReq();
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}
/****************************************************************************************************/
