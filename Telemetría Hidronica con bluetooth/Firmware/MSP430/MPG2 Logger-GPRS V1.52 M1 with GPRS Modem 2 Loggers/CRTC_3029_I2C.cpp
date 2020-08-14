#include"CDevice.h"
#include "stdlib.h"
#include "stdio.h"

#define INT_RTC     BIT1  // Puerto 4.1
#define CLKO        BIT0  // Puerto 4.0
//#define HdwAddrRTC  0x0B

/******************************************************************************/
CRTC_3029_I2C::CRTC_3029_I2C(CEEPROM *memHandler)
{
  P4SEL|=CLKO;    // Habilita entrada de reloj de 1 segundo del RTC

 // P4SEL&=~CLKO;    // Deshabilita entrada de reloj de 1 segundo del RTC

  TACCTL0=CM0|CAP;
  TACCTL0|=CCIE;

   int x;
   this->alarmActivated=false;

   this->getHour_Date(memHandler);
   if(this->m_Buffer[5]==0x00) // Evaluates if Month register is 0x00
   {
   this->initLocalHour_Date(memHandler);
   }
   this->configTimerRTC(memHandler); // Configura RTC I2C con salida de interrupción y salida de reloj a 1 seg
  // this->setRTCOnIntTime(memHandler,mLCD,true);
   //this->setClock(0x01, 0x01, 2007, 0x01, 0x00, 0x00, false); // 1/Enero/2007 01:00:00 Estado inicial. Configura reloj local en microcontrolador
   this->timeZone=-6; //-6
   this->nAlarms=0x18; // 24 alarmas por default
   this->regEnableOp=true;
   for(x=0;x<NAlarms;x++)
   {
      this->alarms[x]=new CAlarms(x);
   }

   this->myTimer=new CTimerA();
   this->startReloj();
}
/******************************************************************************/
CRTC_3029_I2C::~CRTC_3029_I2C()
{}
/******************************************************************************/
void CRTC_3029_I2C::setHour_Date(CEEPROM *memHandler)
{
  memHandler->writeAddr(HdwAddrRTC,0x0008,0x00,this->m_Buffer,7);   // Escribe en RTC vía I2C en registro de reloj
}
/******************************************************************************/
char* CRTC_3029_I2C::getHour_Date(CEEPROM *memHandler)
{
  memHandler->readAddr(HdwAddrRTC,0x0008,0x00,7,this->m_Buffer);  // Lee del RTC vía I2C sobre registro de reloj
  return this->m_Buffer;
}
/******************************************************************************/
void CRTC_3029_I2C::initLocalHour_Date(CEEPROM *memHandler)
{
  /*this->m_Buffer[0]=this->m_Buffer[1]=this->m_Buffer[2]=0x00; // seg=0x00; min=0x00; hour=0x00;
  this->m_Buffer[3]=0x01; // Date (1st)
  this->m_Buffer[4]=0x06; // Days (Week day 6)
  this->m_Buffer[5]=0x01; // January
  this->m_Buffer[6]=0x10; // 2010
  */
 /* this->m_Buffer[0]=0x00; // seg=0
  this->m_Buffer[1]=0x50; // min =50
  this->m_Buffer[2]=0x23; // hour=23;
  this->m_Buffer[3]=0x31; // Date (31st)
  this->m_Buffer[4]=0x01; // Days (Week day 1)
  this->m_Buffer[5]=0x12; // Dec
  this->m_Buffer[6]=0x17; // 2017
  */
  this->m_Buffer[0]=this->m_Buffer[1]=this->m_Buffer[2]=0x00; // seg=0x00; min=0x00; hour=0x00;
  this->m_Buffer[3]=0x01; // Date (1st)
  this->m_Buffer[4]=0x01; // Days (Week day 1)
  this->m_Buffer[5]=0x01; // Jan
  this->m_Buffer[6]=0x17; // 2017
  this->setHour_Date(memHandler);
}
/******************************************************************************/
void CRTC_3029_I2C::configTimerRTC(CEEPROM *memHandler)
{
//  P2DIR&=~CLKO;  // Establece el puerto asignado como entrada.
//  P2OUT|=CLKO;
  //P2IES|=INT_RTC;   // Establece detección de flanco de bajada.
//  P2IES&=~CLKO;   // Establece detección de flanco de subida.
//  P2IE|=CLKO;    // Habilita interrupción del puerto asignado.

//   Buffer[0]=0x11;    // Reset del RTC
//   writeAddr(0x03,0x0004,0x00,Buffer,1);   // Escribe en RTC vía I2C

    //memHandler->readAddr(HdwAddrRTC,0x0000,0x00,5,this->m_Buffer);  // Lee del RTC vía I2C
//  if(Buffer[0]&0x02)
//  {
//    this->m_Buffer[0]=0x99;
//    memHandler->writeAddr(HdwAddrRTC,0x0000,0x00,this->m_Buffer,1);   // Escribe en RTC vía I2C
    this->startRTCCLK(memHandler);
    this->m_Buffer[2]=0x00;
    memHandler->writeAddr(HdwAddrRTC,0x0002,0x00,this->m_Buffer,1);   // Escribe en RTC vía I2C
//  }
  //Buffer[2]=0x20;
  //Buffer[3]=0x00;   // Dato para establecer el dato de los regitros del Timer.
  //writeAddr(0x03,0x0018,0x00,&Buffer[2],2);   // Escribe en RTC vía I2C

  //Buffer[0]|=0x04;
  //Buffer[1]|=0x02;   // Dato para habilitar la interrupción por Timer Interno del RTC.
  //writeAddr(0x03,0x0000,0x00,Buffer,2);   // Escribe en RTC vía I2C

  //Buffer[0]|=0x02;   // Dato para habilitar la interrupción por Timer Interno del RTC.
  //writeAddr(0x03,0x0000,0x00,Buffer,1);   // Escribe en RTC vía I2C


   this->m_Buffer[0]=0x0E;
   memHandler->writeAddr(HdwAddrRTC,0x0030,0x00,this->m_Buffer,1);   // Escribe en RTC vía I2C


}
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
// Rutina que incrementa en segundos el RTC soportando años bisciestos.
void CRTC_3029_I2C::incClock(char nseg)
{
  div_t result;
  result=div(this->year,4);

   this->seg+=nseg;
   if (this->seg>=(char)60)
   {
      this->seg-=60;
      this->min++;
      if (this->min==(char)60)
      {
        this->min=0;
        this->hour++;
        if(this->hour==(char)24)
        {
          this->hour=0;
          this->day++;
          //if(result.rem || myClock->month!=Feb)       // En caso de que no sea año bisciesto o que sea diferente de Febrero.
          if(result.rem || this->month!=numMonth[2])       // En caso de que no sea año bisciesto o que sea diferente de Febrero.
          {
             if(this->day==numDays[this->month-1]+1)
             {
              this->day=1;
              this->month++;
              //if(myClock->month==Dec+1)
              if(this->month==numMonth[12]+1)
              {
                //myClock->month=Jan;
                this->month=numMonth[1];
                this->year++;
              }
             }
          }
          else                                        // En caso de que sea año bisciesto y sea el mes de Febrero
          {
             if(this->day==numDays[this->month-1]+2)
             {
              this->day=1;
              this->month++;
             }
          }
        }
      }
   }
}
/******************************************************************************/
/******************************************************************************/
// Rutina que incrementa en segundos el RTC soportando años bisciestos.
/*void CRTC_3029_I2C::incClockWLCD(CLcd *myLCD, char nseg)
{
   char chrData[5];

  div_t result;
  result=div(this->year,4);


   this->seg+=nseg;

   if (this->seg>=(char)60)
   {
      this->seg-=60;
      this->min++;
      if (this->min==(char)60)
      {
        this->min=0;
        this->hour++;
        if(this->hour==(char)24)
        {
          this->hour=0;
          this->day++;
          //if(result.rem || myClock->month!=Feb)       // En caso de que no sea año bisciesto o que sea diferente de Febrero.
          if(result.rem || this->month!=numMonth[2])       // En caso de que no sea año bisciesto o que sea diferente de Febrero.
          {
             if(this->day==numDays[this->month-1]+1)
             {
              this->day=1;
              this->month++;
              //if(myClock->month==Dec+1)
              if(this->month==numMonth[12]+1)
              {
                //myClock->month=Jan;
                this->month=numMonth[1];
                this->year++;
                sprintf(&chrData[0],"%4d",this->year);
                myLCD->sendMessage(0x8C,4,&chrData[0],0xCC,0,"", false); // Imprime año
              }
              sprintf(&chrData[0],"%2d",this->month);
              if(chrData[0]==0x20)
              chrData[0]=0x30;
              myLCD->sendMessage(0x89,2,&chrData[0],0xCC,0,"", false); // Imprime mes
             }
          }
          else                                        // En caso de que sea año bisciesto y sea el mes de Febrero
          {
             if(this->day==numDays[this->month-1]+2)
             {
              this->day=1;
              this->month++;
              sprintf(&chrData[0],"%2d",this->month);
              if(chrData[0]==0x20)
              chrData[0]=0x30;
              myLCD->sendMessage(0x89,2,&chrData[0],0xCC,0,"", false); // Imprime mes
             }
          }
          sprintf(&chrData[0],"%2d",this->day);
          if(chrData[0]==0x20)
          chrData[0]=0x30;
          myLCD->sendMessage(0x86,2,&chrData[0],0xCC,0,"", false); // Imprime dia
        }
        sprintf(&chrData[0],"%2d",this->hour);
        if(chrData[0]==0x20)
        chrData[0]=0x30;
        myLCD->sendMessage(0xC6,2,&chrData[0],0xCC,0,"", false); // Imprime horas
      }
      sprintf(&chrData[0],"%2d",this->min);
      if(chrData[0]==0x20)
      chrData[0]=0x30;
      myLCD->sendMessage(0xC9,2,&chrData[0],0xCC,0,"", false); // Imprime minutos
   }
   sprintf(&chrData[0],"%2d",this->seg);
   if(chrData[0]==0x20)
   chrData[0]=0x30;
   myLCD->sendMessage(0xCC,2,&chrData[0],0xCC,0,"", false); // Imprime segundos

   myLCD->setAddr(0xD2);
   if(this->getEnabledShedule())
  {
    //      myLCD->setCaracter(0x30+this->getNAlarms());
    sprintf(chrData,"%2d",this->getNAlarms());
    myLCD->setCaracter(chrData[0]);
    myLCD->setCaracter(chrData[1]);
  }
  else
  {
    myLCD->setCaracter('*');
  }

}*/
/******************************************************************************/
/*void CRTC_3029_I2C::setClockWLCD(CLcd *myLCD,char lday, char lmonth, int lyear, char lhour, char lmin, char lseg, bool TZ, bool isUTC)
{
   int hourTemp=0;

   if(TZ)
   {
     hourTemp=lhour;
     if(isUTC)
     {
        hourTemp+=this->timeZone;
     }

    if(hourTemp<0)
    {
     hourTemp+=24;
      lday--;
    }
    else
    {
     if(hourTemp>=24)
      {
        hourTemp-=24;
        lday++;
      }
    }
   }
   else
   {hourTemp=lhour;}

   this->seg=lseg;
   this->min=lmin;
   this->hour=hourTemp;
   this->day=lday;
   this->month=lmonth;
   this->year=lyear;


  char chrData[15];

  myLCD->sendMessage(0x80,19,"Fecha:  /  /     D:",0xC0,18,"Rg:     :  :    H:", true);

  sprintf(&chrData[0],"%2d",this->day);
  if(chrData[0]==0x20)
     chrData[0]=0x30;
  sprintf(&chrData[2],"%2d",this->month);
  if(chrData[2]==0x20)
     chrData[2]=0x30;



  sprintf(&chrData[4],"%4d",this->year);
  sprintf(&chrData[8],"%2d",this->hour);
  if(chrData[8]==0x20)
     chrData[8]=0x30;


  sprintf(&chrData[10],"%2d",this->min);
  if(chrData[10]==0x20)
     chrData[10]=0x30;
  sprintf(&chrData[12],"%2d",this->seg);
  if(chrData[12]==0x20)
     chrData[12]=0x30;

  myLCD->sendMessage(0x86,2,&chrData[0],0x89,2,&chrData[2], false);
  myLCD->sendMessage(0x8C,4,&chrData[4],0xC6,2,&chrData[8], false);
  myLCD->sendMessage(0xC9,2,&chrData[10],0xCC,2,&chrData[12], false);

  myLCD->setAddr(0xD2);
  if(this->getEnabledShedule())
  {
//    myLCD->setCaracter(0x30+this->getNAlarms());
    sprintf(chrData,"%2d",this->getNAlarms());
    myLCD->setCaracter(chrData[0]);
    myLCD->setCaracter(chrData[1]);
  }
  else
  {
    myLCD->setCaracter('*');
  }


}*/
/******************************************************************************/
void CRTC_3029_I2C::setClock(char lday, char lmonth, int lyear, char lhour, char lmin, char lseg, bool TZ, bool isUTC)
{
  int hourTemp=0;
  if(TZ)
   {
    hourTemp=lhour;
     if(isUTC)
     {
        hourTemp+=this->timeZone;
     }

    if(hourTemp<0)
    {
     hourTemp+=24;
      lday--;
    }
    else
    {
     if(hourTemp>=24)
      {
        hourTemp-=24;
        lday++;
      }
    }
   }
   else
   {hourTemp=lhour;}

   this->seg=lseg;
   this->min=lmin;
   this->hour=hourTemp;
   this->day=lday;
   this->month=lmonth;
   this->year=lyear;
}
/******************************************************************************/
int CRTC_3029_I2C::compareRegAlarm()
{
  int x=0;
  int myResult=0;

  if(this->regEnableOp)
  {

    for (x=0;x<this->nAlarms;x++)
    {
      if(this->hour==this->alarms[x]->al_hour && this->min==this->alarms[x]->al_min && (this->alarms[x]->getAlarmFlag())==false && this->alarmActivated==false)
      {
         this->alarms[x]->setAlarmFlag();
         myResult=x+1; this->alarmActivated=true; break;
      }
      else
      {

        if(this->min!=this->alarms[x]->al_min && (this->alarms[x]->getAlarmFlag())==true)
        {
           this->alarms[x]->clearAlarmFlag();
           this->alarmActivated=false;
        }

      }
    }
  }
  else
  {
    for (x=0;x<this->nAlarms;x++)
    {this->alarms[x]->clearAlarmFlag();}
    this->alarmActivated=false;
  }

  return myResult; // devuelve el Id de la alarma activada comenzando desde 1.

}
/******************************************************************************/
bool CRTC_3029_I2C::isHourFlagLoggerStoreForGPRS()
{
  int x=0;
  bool myResult=false;

  for (x=0;x<this->nAlarms;x++) // Qty. of time alarms for GPRS communication
  {
    if(this->hour==this->alarms[x]->al_hour && this->min==this->alarms[x]->al_min)
    {
       myResult=true;
       break;
    }
    else
    {
      if(this->min!=this->alarms[x]->al_min && (this->alarms[x]->getAlarmFlag())==true)
      {
         this->alarms[x]->clearAlarmFlag();
      }
     }
  }

  return myResult; // Returns true if there's a logger to store and send it via GPRS time later

}
/******************************************************************************/
void CRTC_3029_I2C::setAlarm(char nAl, char nHour,char nMin)
{
  this->alarms[nAl]->setAlarm(nHour,nMin);
}
/******************************************************************************/
void CRTC_3029_I2C::startReloj()
{
    this->myTimer->enableTAIE();
}

/******************************************************************************/
void CRTC_3029_I2C::stopReloj()
{
  this->myTimer->disableTAIE();
}
/******************************************************************************/
bool CRTC_3029_I2C::activatedAlarm()
{
  return this->alarmActivated;
}
/******************************************************************************/
void CRTC_3029_I2C::setTimeZone(int Data)
{
  this->timeZone=Data;
}
/******************************************************************************/
int* CRTC_3029_I2C::getTimeZone()
{
  return &this->timeZone;
}
/******************************************************************************/
void CRTC_3029_I2C::setNAlarms(char noAlarms)
{
  this->nAlarms=noAlarms;
}
/******************************************************************************/
char CRTC_3029_I2C::getNAlarms()
{
  return this->nAlarms;
}
/******************************************************************************/
bool CRTC_3029_I2C::getEnabledShedule()
{
  return this->regEnableOp;
}
/******************************************************************************/
/*void CRTC_3029_I2C::setRTCOnIntTime(CEEPROM *memHandler, CLcd *mLCD, bool displayIt, bool isUTC)
{
  this->getHour_Date(memHandler);
  this->seg=((this->m_Buffer[0]&0x70)/0x10)*10 + (this->m_Buffer[0]&0x0F);
  this->min=((this->m_Buffer[1]&0x70)/0x10)*10 + (this->m_Buffer[1]&0x0F);
  this->hour=((this->m_Buffer[2]&0x30)/0x10)*10 + (this->m_Buffer[2]&0x0F);

  this->day=((this->m_Buffer[3]&0x30)/0x10)*10 + (this->m_Buffer[3]&0x0F);
  this->month=((this->m_Buffer[5]&0x10)/0x10)*10 + (this->m_Buffer[5]&0x0F);
  this->year=2000+(((this->m_Buffer[6]&0x70)/0x10)*10 + (this->m_Buffer[6]&0x0F));

  if(displayIt)
  {
    this->setClockWLCD(mLCD,this->day, this->month, this->year, this->hour, this->min, this->seg, true, isUTC);
  }
  else
  {
    this->setClock(this->day, this->month, this->year, this->hour, this->min, this->seg, true, isUTC);
  }
}*/
/******************************************************************************/
void CRTC_3029_I2C::setRTCOnIntTime(CEEPROM *memHandler, bool isUTC)
{
  this->getHour_Date(memHandler);
  this->seg=((this->m_Buffer[0]&0x70)/0x10)*10 + (this->m_Buffer[0]&0x0F);
  this->min=((this->m_Buffer[1]&0x70)/0x10)*10 + (this->m_Buffer[1]&0x0F);
  this->hour=((this->m_Buffer[2]&0x30)/0x10)*10 + (this->m_Buffer[2]&0x0F);

  this->day=((this->m_Buffer[3]&0x30)/0x10)*10 + (this->m_Buffer[3]&0x0F);
  this->month=((this->m_Buffer[5]&0x10)/0x10)*10 + (this->m_Buffer[5]&0x0F);
  this->year=2000+(((this->m_Buffer[6]&0x70)/0x10)*10 + (this->m_Buffer[6]&0x0F));

  this->setClock(this->day, this->month, this->year, this->hour, this->min, this->seg, true, isUTC);
}
/******************************************************************************/
void CRTC_3029_I2C::stopRTCCLK(CEEPROM *memHandler)
{
   this->m_Buffer[0]=0x19;
   memHandler->writeAddr(HdwAddrRTC,0x0000,0x00,this->m_Buffer,1);
}
/******************************************************************************/
void CRTC_3029_I2C::startRTCCLK(CEEPROM *memHandler)
{
   this->m_Buffer[0]=0x99;
   memHandler->writeAddr(HdwAddrRTC,0x0000,0x00,this->m_Buffer,1);
}
/******************************************************************************/
void CRTC_3029_I2C::hexToBcdRTC(int *Buffer)
{
  char var1,var2;

  var1=Buffer[0]/0x0A;
  var2=(var1*0x10)|(Buffer[0]-(var1*10));
  this->m_Buffer[3]=var2;        // Day

  this->m_Buffer[4]=0x01;        // Days (Week day)

  var1=Buffer[1]/0x0A;
  var2=(var1*0x10)|(Buffer[1]-(var1*10));
  this->m_Buffer[5]=var2;         // Month

  var1=(Buffer[2]-2000)/0x0A;
  var2=(var1*0x10)|((Buffer[2]-2000)-(var1*10));
  this->m_Buffer[6]=var2;         // Year from 2000

  var1=Buffer[3]/0x0A;
  var2=(var1*0x10)|(Buffer[3]-(var1*10));
  this->m_Buffer[2]=var2;         // Hour

  var1=Buffer[4]/0x0A;
  var2=(var1*0x10)|(Buffer[4]-(var1*10));
  this->m_Buffer[1]=var2;         // Min

  var1=Buffer[5]/0x0A;
  var2=(var1*0x10)|(Buffer[5]-(var1*10));
  this->m_Buffer[0]=var2;         // Seg
}
/******************************************************************************/
/*void CRTC_3029_I2C::updateLCDDate(CLcd *myLCD)
{
  char chrData[15];

  myLCD->sendMessage(0x80,19,"Fecha:  /  /     D:",0xC0,18,"Rg:     :  :    H:", true);

  sprintf(&chrData[0],"%2d",this->day);
  if(chrData[0]==0x20)
     chrData[0]=0x30;
  sprintf(&chrData[2],"%2d",this->month);
  if(chrData[2]==0x20)
     chrData[2]=0x30;



  sprintf(&chrData[4],"%4d",this->year);
  sprintf(&chrData[8],"%2d",this->hour);
  if(chrData[8]==0x20)
     chrData[8]=0x30;


  sprintf(&chrData[10],"%2d",this->min);
  if(chrData[10]==0x20)
     chrData[10]=0x30;
  sprintf(&chrData[12],"%2d",this->seg);
  if(chrData[12]==0x20)
     chrData[12]=0x30;

  myLCD->sendMessage(0x86,2,&chrData[0],0x89,2,&chrData[2], false);
  myLCD->sendMessage(0x8C,4,&chrData[4],0xC6,2,&chrData[8], false);
  myLCD->sendMessage(0xC9,2,&chrData[10],0xCC,2,&chrData[12], false);

}*/
/******************************************************************************/



