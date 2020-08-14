#include "CController.h"

// P6.0 es utilizado para entrada analógica al microcontolador para medir
// voltaje de la batería proveniente del divisor de voltaje
/****************************************************************************************************/
CADC12::CADC12()
{}
/***************************************************************************************************/
CADC12::~CADC12()
{}
/****************************************************************************************************/
void CADC12::configADC12(int NChannels)
{
  int x=0;
  unsigned int reg=1;
  unsigned char *pADCMCTL;


  ADC12CTL0 = SHT0_0 | REFON | ADC12ON | REF2_5V | MSC;
  ADC12CTL1 = SHP | CONSEQ_3;

  pADCMCTL=(unsigned char *)&ADC12MCTL0;

  if (NChannels)
  {
    for (x=0;x<NChannels;x++)
    {
      *pADCMCTL=SREF_1 + (reg-1);
      reg++;
      pADCMCTL++;
    }
    pADCMCTL--;
    *pADCMCTL+=EOS;
    reg--;
    P6SEL=reg;
    //P6SEL=0x07;
    ADC12IE = reg;
    //ADC12IE = 0x07;
    ADC12CTL0 |= ENC;
  }
  else
  {
    P6SEL=reg-1;
    this->stopADC12();
  }

}
/****************************************************************************************************/
void CADC12::startADC12()
{
   ADC12CTL0 |= (REFON | ADC12ON | REF2_5V);
   ADC12IFG=0x0000;
   ADC12CTL1 |= (SHP | CONSEQ_3);
   ADC12CTL0 |= ENC;
   ADC12CTL0 |= ADC12SC;
   ADC12IE|=0x01; // Prueba para canal 0
}
/****************************************************************************************************/
void CADC12::stopADC12()
{
   ADC12IFG=0x0000;
   ADC12CTL0 &= ~ENC;
   ADC12CTL1 &= ~(SHP | CONSEQ_3);
   ADC12CTL0 &= ~ADC12SC;
   ADC12CTL0 &= ~(REFON | ADC12ON | REF2_5V);
   ADC12IE&=~0x01; // Prueba para canal 0
}
/****************************************************************************************************/
void CADC12::clearADCflags()
{
  ADC12IFG=0x0000;
}
/****************************************************************************************************/
char CADC12::isFlgLogger(struct ADC12* RecADC12)
{
  return this->flagLogger;
}
/****************************************************************************************************/
void CADC12::setFlgLogger(struct ADC12* RecADC12)
{
  this->flagLogger|=0x01;
}
/****************************************************************************************************/
void CADC12::clearFlgLogger(struct ADC12* RecADC12)
{
  this->flagLogger&=~0x01;
}
/****************************************************************************************************/
