#include "CAlarms.h"

CAlarms::CAlarms(int hour)
{
  this-> al_hour=hour;
  this-> al_min=0x00;
  this-> al_seg=0x00;
  //this->alarm_on=false;
  this->alarm_activated=false;
}
/******************************************************************************/
CAlarms::~CAlarms()
{

}
/******************************************************************************/
void CAlarms::setAlarm(char nHour,char nMin)
{
  this-> al_hour=nHour;
  this-> al_min=nMin;
  this-> al_seg=0x00;
}
/******************************************************************************/
void CAlarms::offsetAlarm(char oHour,char oMin,char offsetHour,char offsetMin) // Establece la alarma horas y minutos adelante (ffsetHor y offsetMin) de la hora actual (oHour y oMin).
{
      this->setAlarm(oHour,oMin);

      this->al_min+=offsetMin;
      if (this->al_min>=(char)60)
      {
        this->al_min-=60;
        offsetHour++;
      }

      this->al_hour+=offsetHour;
      if(this->al_hour>=(char)24)
      {
        this->al_hour-=24;
      }
}
/******************************************************************************/
void CAlarms::setAlarmFlag()
{
   this->alarm_activated=true;
}
/******************************************************************************/
void CAlarms::clearAlarmFlag()
{
   this->alarm_activated=false;
}
/******************************************************************************/
bool CAlarms::getAlarmFlag()
{
   return this->alarm_activated;
}
/******************************************************************************/
