#include "CDevice.h"
#include "stdlib.h"

#define limitByMem        0x20000        // 128 KB
#define limitByPage       0x7F           // 128 Bytes
//#define limitTotalMemAddr 0x0B           // Dirección física de memoria de 128 Kbytes (ver CEEPROM)
//#define nDataLogg         32           // Cantidad de datos de 8 bits a almacenar


//const char timeLogger[34]={0,10,20,30,1,2,5,10,15,30,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
const char timeLoggerSat[12]={0,15,30,1,2,3,4,6,8,12,24};
//const char timeLoggerSat[12]={0,2,30,1,2,3,4,6,8,12,24};
const char msgSmplTime[3][4]={"seg","min","hrs"};
//const char msgStateEnDis[][]={"Habilitar?", "Desh"};
/****************************************************************************************************/
CLogger::CLogger()
{
  //this->reqFromLogger=false;       // Bandera de identificación de que el logger ha cumplido el tiempo de muestreo
  this->enabledLogg=true;          // Bandera logger Deshabilitado por default.
  this->fullLoggMem=false;         // Bandera de memoria llena
  this->alarmLoggSatActivated=false; // badera de detección de horario de ejecución de la alarma de recolección para obtener infro del medidor
//  this->countSampleTime=0;         // Inicio de contador a 0.
  //this->IDSampleTime=1;            // Inicio de sample time a 15 min
  this->IDSampleTime=3;            // Inicio de sample time a 1 hr
  this->nData=0;
  this->flagReqStoreData=false;
}
/****************************************************************************************************/
void CLogger::setLimitAddrMem(char Addr)
{
  this->limitTotalMemAddr=Addr;
}
/****************************************************************************************************/
/*void CLogger::setPointerWrite(unsigned long int pWrMem)
{
  this->p_memLoggerWrite=pWrMem;
}*/
/****************************************************************************************************/
/*unsigned long int *CLogger::getPointerWrite()
{
  return &this->p_memLoggerWrite;
}*/
/****************************************************************************************************/
/****************************************************************************************************/
void CLogger::enableLogg()
{
  this->enabledLogg=true;
}
/****************************************************************************************************/
void CLogger::disableLogg()
{
  this->enabledLogg=false;
}
/****************************************************************************************************/
bool CLogger::isEnabledLogg()
{
  return this->enabledLogg;
}
/****************************************************************************************************/
/*  void CLogger::setDefSampleTime(char *Buffer)
  {
    *(unsigned long int*)&Buffer[0]=(unsigned long int)*this->getTimeLoggerSatByIndex(this->IDSampleTime);
     switch(this->IDSampleTime)
     {
     case 0:
            break;
     case 1:
     case 2:
       *(unsigned long int*)&Buffer[0]*=60;
            break;
     default:
            *(unsigned long int*)&Buffer[0]*=3600;
            break;
     }


    this->sampleTime=*(unsigned long int*)&Buffer[0];
  }*/
/****************************************************************************************************/
/*  unsigned long int *CLogger::getDefSampleTime()
  {
    return &this->sampleTime;
  }*/
/****************************************************************************************************/
/*  void CLogger::incCounterSmplTime()
  {
    this->countSampleTime++;
  }*/
/****************************************************************************************************/
/*void CLogger::clearCounterSmplTime()
  {
    this->countSampleTime=0;
  }
*/
/****************************************************************************************************/
/*  unsigned long int *CLogger::getCounterSmplTime()
  {
    return &this->countSampleTime;
  }*/
/****************************************************************************************************/
  void CLogger::loggingData(unsigned long int pAddr, char *Buffer, int nDataLogg, CEEPROM *memHandler) // Solo para una memoria de 128 KB
  {
    char DevAddress=0x00;
    unsigned long int p1,p2;

    unsigned long int p_memLoggerWrite=pAddr;

        DevAddress= p_memLoggerWrite/limitByMem;
        if(DevAddress<this->limitTotalMemAddr)                  // Verifica si se ha llenado la memoria total del logger interno
        {

          pAddr=p_memLoggerWrite+nDataLogg;               // Apuntador de memoria actual más cantidad de bytes a almacenar en memoria
          pAddr&=limitByPage;

          if(pAddr < (p_memLoggerWrite&limitByPage) && pAddr!=0x00)
          {// Agregar código para almacenar datos en dos bloques separados
            p2=pAddr;
            p1=nDataLogg-p2;                              // Cantidad de datos a almacenar menos los que superaron el limite de 128 Bytes pr página
            pAddr=p_memLoggerWrite & (limitByMem-1);

            memHandler->writeAddr(DevAddress,pAddr, 0x01, Buffer, p1);
            Buffer+=p1;
            pAddr+=p1;
            if(pAddr & limitByMem)
            {
              DevAddress++;
              pAddr&=(limitByMem-1);
            }
            if(DevAddress<this->limitTotalMemAddr)
            {
              memHandler->writeAddr(DevAddress,pAddr, 0x01, Buffer, p2);
              //this->p_memLoggerWrite+=nDataLogg;                 // Incrementa el apuntador en la cantidad de datos almacenados
            }
            else
            {
              this->fullLoggMem=true;
            }
          }
          else
          {
            pAddr=p_memLoggerWrite & (limitByMem-1);
            memHandler->writeAddr(DevAddress,pAddr, 0x01, Buffer, nDataLogg);
            //this->p_memLoggerWrite+=nDataLogg;                // Incrementa el apuntador en la cantidad de datos almacenados
          }
        }
        else
        {
          this->fullLoggMem=true;
        }

  }
/****************************************************************************************************/
void CLogger::readLoggedData(unsigned long int pAddr, char *Buffer, CEEPROM *memHandler) // Solo para una memoria de 128 KB
  {
    char DevAddress=0x00;

    unsigned long int p_memLoggerRead=pAddr;

    DevAddress = p_memLoggerRead/limitByMem;                // Limit by mem = 0x20000 = 128 KBytes = 131,072 bytes

    if(DevAddress<this->limitTotalMemAddr)                  // Verifica si se ha llenado la memoria total del logger interno
    {
      pAddr=p_memLoggerRead & (limitByMem-1);
      memHandler->readAddr(DevAddress,pAddr, 0x01, (limitByPage+1), Buffer);
    }
  }
/****************************************************************************************************/
/*bool CLogger::isTimeSampleOn()    // Funcion para  detectar si el tiempo de muestreo se ha cumplido
{
  if (this->countSampleTime>=86400) // Si es igual a 24 hrs
  {
      this->countSampleTime=0;
  }

  if(this->sampleTime)  // Si es diferente de cero
  {
    if(this->countSampleTime>=this->sampleTime)
    {
      this->countSampleTime=0;
      this->reqFromLogger=true;
      return true;
    }
  }
  else    // En caso de que sea cero (24 hrs)
  {
    if(this->countSampleTime<=this->sampleTime)
    {
      this->reqFromLogger=true;
      return true;
    }
  }
  return false;
}*/
/******************************************************************************/
int CLogger::isTimeSampleOn(CRTC_3029_I2C* Reloj)
{
  int x=0;
  int myResult=0;
  div_t l_temp;


  switch(this->IDSampleTime)
  {
  case 0:
          break;
  case 1:
  case 2:
            l_temp=div(Reloj->min,timeLoggerSat[this->IDSampleTime]);
            if(!l_temp.rem)
            {
              if(!this->alarmLoggSatActivated)
              {
                myResult=x+1; this->alarmLoggSatActivated=true;
              }
            }
            else
            {
              this->alarmLoggSatActivated=false;
            }
          break;
  default:
          if(!Reloj->min)
          {
            if(!this->alarmLoggSatActivated)
            {
              l_temp=div(Reloj->hour,timeLoggerSat[this->IDSampleTime]);
              if(!l_temp.rem)
              {
                myResult=x+1; this->alarmLoggSatActivated=true;
              }
            }
          }
          else
          {
            this->alarmLoggSatActivated=false;
          }
          break;
  }

  return myResult; // devuelve el Id de la alarma activada comenzando desde 1.

}
/******************************************************************************/
/****************************************************************************************************/
/*
bool CLogger::isReqLogger()
{
  return this->reqFromLogger;
}*/
/****************************************************************************************************/
/*void CLogger::setReqLoggerFlg()
{
  this->reqFromLogger=true;
}*/
/****************************************************************************************************/
/*void CLogger::clearReqLoggerFlg()
{
  this->reqFromLogger=false;
}*/
/****************************************************************************************************/
bool CLogger::isMemFull()
{
  return this->fullLoggMem;
}
/****************************************************************************************************/
bool CLogger::readLoggDataByBlk(char *Buffer, CEEPROM *memHandler, int NBlock) // Solo para una memoria de 128 KB
{
  char DevAddress=0x00;
  //unsigned long int pAddr;
       this->p_memLoggerRead=((unsigned long int)NBlock*(limitByPage+1))-(limitByPage+1); // Dirección de inicio de lectura.
       DevAddress= p_memLoggerRead/limitByMem;
      if(DevAddress<this->limitTotalMemAddr)                  // Verifica si se ha llegado al fin de la lectrura de la memoria
      {
          this->p_memLoggerRead&=(limitByMem-1);
          memHandler->readAddr(DevAddress,this->p_memLoggerRead, 0x01, (limitByPage+1), Buffer);
          return true;
      }
      else
      {
        return false;
      }
}
/****************************************************************************************************/
void CLogger::copyDateTime(CRTC_3029_I2C *Reloj)
{
  this->timeRegCopy=*(double*)&Reloj->day;
}
/****************************************************************************************************/
double *CLogger::getCopiedDateTime()
{
  return &this->timeRegCopy;
}
/****************************************************************************************************/
/*char * CLogger::getTimeLoggerByIndex(int index)
{
  return (char*)&timeLogger[index];
}*/
/****************************************************************************************************/
char* CLogger::getTimeLoggerSatByIndex(int index)
{
  return (char*)&timeLoggerSat[index];
}
/****************************************************************************************************/
char* CLogger::getTimeUnitsByIndex(int index)
{
  return (char*)&msgSmplTime[index];
}
/****************************************************************************************************/
void CLogger::setIndexSampleTime(char index)
{
  this->IDSampleTime=index;
}
/****************************************************************************************************/
char* CLogger::getIndexSampleTime()
{
  return &this->IDSampleTime;
}
/****************************************************************************************************/
char* CLogger::getEnabledLoggAddr()
{
  return (char*)&this->enabledLogg;
}
/****************************************************************************************************/
void CLogger::clearNData()
{
 this->nData=0;
}
/****************************************************************************************************/
void CLogger::incNData()
{
 this->nData++;
}
/****************************************************************************************************/
char CLogger::getNDataBCD()
{
  int temp1;
  char returndata=0x00;

  if(this->nData!=0x8000)
  {
    temp1=this->nData;
    temp1/=10;

    returndata=(temp1*0x10);
    temp1=this->nData-(temp1*10);
    returndata|=temp1;
  }
  else
  {
    returndata=0xFF;
  }

 return returndata;
}
/****************************************************************************************************/
int* CLogger::getPNDataBCD()
{
  return &this->nData;
}
/****************************************************************************************************/
void CLogger::setNDataBrkLnk()
{
 this->nData=0x8000;
}
/****************************************************************************************************/
bool CLogger::isFlagStoreLogger()
{
  return this->flagReqStoreData;
}
/****************************************************************************************************/
void CLogger::clearFlagStoreLogger()
{
  this->flagReqStoreData=false;
}
/****************************************************************************************************/
void CLogger::setFlagStoreLogger()
{
  this->flagReqStoreData=true;
}
/****************************************************************************************************/


