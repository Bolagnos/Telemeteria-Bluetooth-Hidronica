#include "CDevice.h"
#include "stdlib.h"

/****************************************************************************/
CModemUTCClk::CModemUTCClk()
{
  this->year=0;
  this->dayweek=this->month=this->day=this->hour=this->min=this->seg=0x00;
}
/****************************************************************************/
CModemUTCClk::~CModemUTCClk()
{
}
/****************************************************************************/
const int T_DaysInFourYears [5] ={0,366,731,1096,1461};
// Cantidad de dias acumulados por mes a�o normal (365 d�as)
const int T_DaysInNormMonth [13] ={0,31,59,90,120,151,181,212,243,273,304,334,365};
// Cantidad de dias acumulados por mes a�o bisciesto (366 d�as)
const int T_DaysInBiscMonth [13] ={0,31,60,91,121,152,182,213,244,274,305,335,366};

void CModemUTCClk::calcDate(unsigned int nWeeks, unsigned long nSeconds)
{
unsigned long l_days=6;
int x;
int sum_a�os=1980;
int  l_month=0;

if(nWeeks)
{

  l_days=7*(nWeeks);

// Cada 28 a�os se repiten las fechas por lo tanto 28 a�os es igual a 10227 dias (contando a�os bisciestos).
  ldiv_t prev_a�os= div(l_days,(long)10227);

  sum_a�os=prev_a�os.quot*28;
  sum_a�os+=1980;

// 4 a�os contienen 1461 d�as (incluyendo el a�o bisciesto).
  prev_a�os=div(prev_a�os.rem,(long)1461);

  sum_a�os+=(prev_a�os.quot*4);

  for(x=0;x<4;x++)
  {
    if(prev_a�os.rem>T_DaysInFourYears[x] && prev_a�os.rem<=T_DaysInFourYears[x+1])
    {
      break;
    }
  }
  sum_a�os+=x;
  l_days=(prev_a�os.rem-T_DaysInFourYears[x]);
  if(x)
  {
    for(x=0;x<12;x++)
    {
      if(l_days>T_DaysInNormMonth[x] && l_days<=T_DaysInNormMonth[x+1])
      {
        break;
      }
    }
    l_days-=T_DaysInNormMonth[x];
    this->dayweek=this->calcDay_Hours(nSeconds);
    l_days+=(6 + this->dayweek);
    if(l_days>T_DaysInNormMonth[x+1]-T_DaysInNormMonth[x])
    {
      l_days-=(T_DaysInNormMonth[x+1]-T_DaysInNormMonth[x]);
      x++;
    }
  }
  else
  {
    for(x=0;x<12;x++)
    {
      if(l_days>T_DaysInBiscMonth[x] && l_days<=T_DaysInBiscMonth[x+1])
      {
        break;
      }
    }
    l_days-=T_DaysInBiscMonth[x];
    this->dayweek=this->calcDay_Hours(nSeconds);
    l_days+=(6 + this->dayweek);
    if(l_days>T_DaysInBiscMonth[x+1]-T_DaysInBiscMonth[x])
    {
      l_days-=(T_DaysInBiscMonth[x+1]-T_DaysInBiscMonth[x]);
      x++;
    }
  }

  l_month=x;  // 0=enero, 11=Diciembre

}

if(l_month>=12)
{
  l_month=0;
  sum_a�os++;
}

  this->day=l_days;
  this->year=sum_a�os;
  this->month=l_month;



}
/****************************************************************************/
int CModemUTCClk::calcDay_Hours(unsigned long nSeconds)
{
  ldiv_t l_reg = div(nSeconds,(long)60);
  this->seg = l_reg.rem;
  l_reg = div(l_reg.quot,(long)60);
  this->min = l_reg.rem;
   l_reg = div(l_reg.quot,(long)24);
  this->hour = l_reg.rem;

  return l_reg.quot;
}
/****************************************************************************/
