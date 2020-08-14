#include "CDevice.h"
#include "stdlib.h"
#include "stdio.h"

CReloj::CReloj()
{
   int x;
   this->alarmActivated=false;
   this->setClock(0x01, 0x01, 2007, 0x01, 0x00, 0x00, false); // 1/Enero/2007 01:00:00 Estado inicial.
   this->timeZone=-6;
   this->nAlarms=0x00;
   this->regEnableOp=true;
   for(x=0;x<NAlarms;x++)
   {
      this->alarms[x]=new CAlarms();
   }
//    this->myTimer=new CTimerB();
}
/******************************************************************************/
CReloj::~CReloj()
{}
/******************************************************************************/
// Tablas

//enum numMonth {Jan=1, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec} nMonth;
//const char numMonth[]= {0,1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
//const char numDays[]= {31,28,31,30,31,30,31,31,30,31,30,31};

/******************************************************************************/
// Definición de funciones.
//void incClock(struct RTC *myClock, char nseg);
void CReloj::incClock(char nseg)
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
void CReloj::incClockWLCD(CLcd *myLCD, char nseg)
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

   myLCD->setAddr(0xD3);
   if(this->getEnabledShedule())
  {
      myLCD->setCaracter(0x30+this->getNAlarms());
  }
  else
  {
    myLCD->setCaracter('*');
  }
}
/******************************************************************************/
void CReloj::setClockWLCD(CLcd *myLCD,char lday, char lmonth, int lyear, char lhour, char lmin, char lseg, bool TZ)
{
   int hourTemp=0;

   if(TZ)
   {
    hourTemp=lhour+this->timeZone;

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

  myLCD->sendMessage(0x80,19,"Fecha:  /  /     D:",0xC0,19,"Rg:     :  :     H:", true);

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

  myLCD->setAddr(0xD3);
  if(this->getEnabledShedule())
  {
      myLCD->setCaracter(0x30+this->getNAlarms());
  }
  else
  {
    myLCD->setCaracter('*');
  }


}
/******************************************************************************/
void CReloj::setClock(char lday, char lmonth, int lyear, char lhour, char lmin, char lseg, bool TZ)
{
  int hourTemp=0;
  if(TZ)
   {
    hourTemp=lhour+this->timeZone;

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
int CReloj::compareRegAlarm()
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
void CReloj::setAlarm(char nAl, char nHour,char nMin)
{
  this->alarms[nAl]->setAlarm(nHour,nMin);
}
/******************************************************************************/
void CReloj::startReloj()
{
    this->myTimer->enableTBIE();
}

/******************************************************************************/
void CReloj::stopReloj()
{
  this->myTimer->disableTBIE();
}
/******************************************************************************/
bool CReloj::activatedAlarm()
{
  return this->alarmActivated;
}
/******************************************************************************/
void CReloj::setTimeZone(int Data)
{
  this->timeZone=Data;
}
/******************************************************************************/
void CReloj::setNAlarms(char noAlarms)
{
  this->nAlarms=noAlarms;
}
/******************************************************************************/
char CReloj::getNAlarms()
{
  return this->nAlarms;
}
/******************************************************************************/
bool CReloj::getEnabledShedule()
{
  return this->regEnableOp;
}
/******************************************************************************/
