#include"CDevice.h"

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
//#define dataFlagEEPROMAddr 0x0000 // Dirección de la bandera de detección de datos en EEPROM.
//#define dataMemID 0x0A5           // Dato de identifiación de la existencia de datos validos en memeoria EEPROM.
//#define logicAddrEEPROM 0x00      // Dirección lógica de la memoria EEPROM.
/******************************************************************************/
#define SDA_1 BIT4  // P4.4
#define SCL_1 BIT5  // P4.5
#define SDA_2 BIT6  // P4.6
#define SCL_2 BIT7  // P4.7
#define SDA_3 BIT2  // P4.2
#define SCL_3 BIT3  // P4.3

/******************************************************************************/
CEEPROM::CEEPROM()
{
  this->secCounter=0;
  this->flagsMem = BIT0; // Selects 1st bank of 512 bytes
  this->clearNewRegMPFlag();
  this->initializeEEPROM();
}
/******************************************************************************/
CEEPROM::~CEEPROM()
{}
/******************************************************************************/
void CEEPROM::initializeEEPROM()
{
   P4DIR&=~(SDA_1|SCL_1|SDA_2|SCL_2|SDA_3|SCL_3); // PONE EL BUS EN 1
   P4OUT&=~(SDA_1|SCL_1|SDA_2|SCL_2|SDA_3|SCL_3); // PONE LA SALIDA EN 0
}
/******************************************************************************/
/*
devaddr -> Dirección fisica del dispositivo externo I2C (EEPROM), 0x00-0x03 banco 1, 0x04-0x07 banco 2, 0x08-0x0B banco 3 (con RTC)
startAdrr -> Direccion de inicio del registro o bloque a escribir sobre la memoria EEPROM.
info -> Dirección de inicio de los datos a respaldar en memoria EEPROM.
length -> Cantidad de bytes a escribir de la memoria EEPROM.
*/

int CEEPROM::writeAddr(char devaddr, unsigned long int startAddr, char flagMSB, char *info, unsigned int length)
{
   char timeoutACK=0;
   char addrDev=0;
   int x=0;
   char bancoMem=0;

   if(devaddr&0x08) // Verifica si se solicita el banco de memoria con RTC
   {
     bancoMem+=2;
   }
   else if(devaddr&0x04) // Verifica si se solicita el segundo banco de memoria
   {
     bancoMem++;
   }

   if(startAddr&0x10000) // Solicitud de segunda página interna de la memoria?
   {
    addrDev=0xA0|(((devaddr&0x03)*4)|0x02);
   }
   else
   {
     addrDev=0xA0|((devaddr&0x03)*4);
   }

   char startAddrMSB=(startAddr/0x0100)&0x00FF;
   char startAddrLSB=startAddr&0x0FF;

   char myACK=1;
   while(myACK && timeoutACK<20)
   {
   startEEPROM(bancoMem);
   sendData((char)addrDev,bancoMem);
   myACK=getACK(bancoMem);
   timeoutACK++;
   }
   if (timeoutACK>=20)
   {
      return 0;     // Activa Error de time out del respuesta de la memoria EEPROM
   }

   if(flagMSB)
   {
      timeoutACK=0;
      myACK=1;
      while(myACK && timeoutACK<20)
      {
        sendData((char)startAddrMSB,bancoMem);
        myACK=getACK(bancoMem);
        timeoutACK++;
      }
      if(timeoutACK>=20)
      {
        return 0;
      }
   }

   timeoutACK=0;
   myACK=1;
   while(myACK && timeoutACK<20)
   {
   sendData((char)startAddrLSB,bancoMem);
   myACK=getACK(bancoMem);
   timeoutACK++;
   }
   if(timeoutACK>=20)
   {
     return 0;
   }

   while(x!=length)
   {
     myACK=1;
     timeoutACK=0;
     while(myACK && timeoutACK<20)
     {
      sendData((char)*info++,bancoMem);
      myACK=getACK(bancoMem);
      timeoutACK++;
     }
     if(timeoutACK>=20)
     {
      return 0;
     }
     x++;
   }

   stopEEPROM(bancoMem);

   return 1;      // Escritura de memoria EERPOM exitosa.
}
/******************************************************************************/
void CEEPROM::startEEPROM(char banco)
{
   char SDA;
   char SCL;

   switch(banco)
   {
   case 1:  SDA=SDA_2; SCL=SCL_2; break;
   case 2:  SDA=SDA_3; SCL=SCL_3; break;
   case 0:
   default: SDA=SDA_1; SCL=SCL_1; break;
   }

   P4DIR&=~(SDA|SCL); // PONE EL BUS EN 1
   P4DIR|=SDA;        // PONE SDA EN 0
   P4DIR|=SCL;        // PONE SCL EN 0
}
/******************************************************************************/
void CEEPROM::stopEEPROM(char banco)
{
   char SDA;
   char SCL;

   switch(banco)
   {
   case 1:  SDA=SDA_2; SCL=SCL_2; break;
   case 2:  SDA=SDA_3; SCL=SCL_3; break;
   case 0:
   default: SDA=SDA_1; SCL=SCL_1; break;
   }

   P4DIR|=SCL;        // PONE SCL EN 0
   P4DIR|=SDA;        // PONE SDA EN 0
   P4DIR&=~SCL;       // PONE SCL EN 1
   P4DIR&=~SDA;       // PONE SDA EN 1
}
/******************************************************************************/
void CEEPROM::sendBit(char cbit, char banco)
{
   char SDA;
   char SCL;

   switch(banco)
   {
   case 1:  SDA=SDA_2; SCL=SCL_2; break;
   case 2:  SDA=SDA_3; SCL=SCL_3; break;
   case 0:
   default: SDA=SDA_1; SCL=SCL_1; break;
   }

   if((cbit & 0x01))
   {
     P4DIR&=~SDA;       // Envia 1 en SDA
   }
   else
   {
     P4DIR|=SDA;        // Envia 0 en SDA
   }

   P4DIR&=~SCL;         // PONE SCL EN 1
   _NOP();
//   _NOP();
   P4DIR|=SCL;          // PONE SCL EN 0
}
/*****************************************************************************/
void CEEPROM::sendData(char data, char banco)
{
   for(int x=0; x<8; x++)
   {
      data<<=1;
      sendBit(__get_SR_register(),banco);
   }
}
/******************************************************************************/
char CEEPROM::getACK(char banco)
{
   char SDA;
   char SCL;

   switch(banco)
   {
   case 1:  SDA=SDA_2; SCL=SCL_2; break;
   case 2:  SDA=SDA_3; SCL=SCL_3; break;
   case 0:
   default: SDA=SDA_1; SCL=SCL_1; break;
   }

   P4DIR&=~SDA;        // PONE EL PIN DE DATOS COMO ENTRADA
   P4DIR&=~SCL;       // PONE SCL EN 1
   char  my_val=P4IN&SDA;
   _NOP();
//   _NOP();
   P4DIR|=SCL;        // PONE SCL EN 0
   return my_val;
}
/******************************************************************************/
/******************************************************************************/
char CEEPROM::readBit(char banco)
{
  char mybit=0;

   char SDA;
   char SCL;

   switch(banco)
   {
   case 1:  SDA=SDA_2; SCL=SCL_2; break;
   case 2:  SDA=SDA_3; SCL=SCL_3; break;
   case 0:
   default: SDA=SDA_1; SCL=SCL_1; break;
   }

   P4DIR&=~SCL;         // PONE SCL EN 1
   if(P4IN&SDA)
   {
      mybit++;
   }
   _NOP();
//   _NOP();
   P4DIR|=SCL;          // PONE SCL EN 0
   return mybit;
}
/******************************************************************************/
char CEEPROM::readData(char banco)
{
   char data=0;
   for(int x=0; x<8; x++)
   {
      data<<=1;
      data|=readBit(banco);
   }
   return data;
}
/******************************************************************************/
void CEEPROM::setACK(char typeACK, char banco)
{
   char SDA;
   char SCL;

   switch(banco)
   {
   case 1:  SDA=SDA_2; SCL=SCL_2; break;
   case 2:  SDA=SDA_3; SCL=SCL_3; break;
   case 0:
   default: SDA=SDA_1; SCL=SCL_1; break;
   }

  if(typeACK)
  {
   P4DIR&=~SDA;        // Pone SDA en 1
  }
  else
  {
    P4DIR|=SDA;       // pone SDA en 0
  }
   P4DIR&=~SCL;       // Pone SCL EN 1
   _NOP();
//   _NOP();
   P4DIR|=SCL;        // Pone SCL EN 0
   P4DIR&=~SDA;        // Pone SDA en 1
}
/******************************************************************************/

/******************************************************************************/
/******************************************************************************/
/*
devaddr -> Dirección fisica del dispositivo externo I2C (EEPROM).
startAdrr -> Direccion de inicio del registro o bloque a leer en la memoria EEPROM
length -> Cantidad de bytes a leer de la memoria EEPROM.
Buffer -> Dirección de inicio de almacenamiento de los datos a obtener.
*/
char* CEEPROM::readAddr(char devaddr, unsigned long int startAddr, char flagMSB, unsigned int length, char* Buffer)
{
   char timeoutACK=0;
   char addrDev=0;
   int x=0;
   char bancoMem=0;

   if(devaddr&0x08) // Verifica si se solicita el segundo banco de memoria
   {
     bancoMem+=2;
   }
   else if(devaddr&0x04) // Verifica si se solicita el segundo banco de memoria
   {
     bancoMem++;
   }

   if(startAddr&0x10000) // Solicitud de segunda página interna de la memoria?
   {
    addrDev=0xA0|(((devaddr&0x03)*4)|0x02);
   }
   else
   {
     addrDev=0xA0|((devaddr&0x03)*4);
   }

   char startAddrMSB=(startAddr/0x0100)&0x00FF;
   char startAddrLSB=startAddr&0x0FF;
// Dummie
   char myACK=1;
   while(myACK && timeoutACK<20)
   {
   startEEPROM(bancoMem);
   sendData((char)addrDev,bancoMem);
   myACK=getACK(bancoMem);
   timeoutACK++;
   }
   if (timeoutACK>=20)
   {
      return Buffer='\0';     // Activa Error de time out del respuesta de la memoria EEPROM
   }

   if(flagMSB)
   {
      timeoutACK=0;
      myACK=1;
      while(myACK && timeoutACK<20)
      {
        sendData((char)startAddrMSB,bancoMem);
        myACK=getACK(bancoMem);
        timeoutACK++;
      }
      if(timeoutACK>=20)
      {
        return Buffer='\0';
      }
   }

   timeoutACK=0;
   myACK=1;
   while(myACK && timeoutACK<20)
   {
    sendData((char)startAddrLSB,bancoMem);
    myACK=getACK(bancoMem);
    timeoutACK++;
   }
   if(timeoutACK>=20)
   {
     return Buffer='\0';
   }
// Start Read
   addrDev|=0x01;       // Read

   timeoutACK=0;
   myACK=1;
   while(myACK && timeoutACK<20)
   {
   startEEPROM(bancoMem);
   sendData((char)addrDev,bancoMem);
   myACK=getACK(bancoMem);
   timeoutACK++;
   }
   if(timeoutACK>=20)
   {
     return Buffer='\0';
   }

   while(1)
   {
     Buffer[x]=readData(bancoMem);//rData;    // Linea de prueba
     x++;
     if (x==length)
     {
       setACK(1,bancoMem);
       break;
     }
     setACK(0,bancoMem);
   }

   stopEEPROM(bancoMem);
   return Buffer;          // Lectura de memoria EEPROM exitosa.
}
/******************************************************************************/
// bank = 0 Selects bank 1 (512 bytes size)
// bank = 1 Selects bank 2 (512 bytes size)
void CEEPROM::setsBank(int bank)
{
  this->flagsMem&=~(BIT0|BIT1);
  switch(bank)
  {
  case BIT1:
          this->flagsMem |= BIT1; // Selects 1st bank of 512 bytes
          break;
  case BIT0:
          this->flagsMem |= BIT0; // Selects 1st bank of 512 bytes
  default:
          break;
  }
}
/******************************************************************************/
int CEEPROM::getBank()
{
  return (this->flagsMem & (BIT0 | BIT1));
}
/******************************************************************************/
void CEEPROM::setChangeFlagBank()
{
  this->flagsMem|=flagChgBank;
}
/******************************************************************************/
void CEEPROM::clearChangeFlagBank()
{
  this->flagsMem&=~flagChgBank;
}
/******************************************************************************/
bool CEEPROM::getChangeFlagBank()
{
  if(this->flagsMem & flagChgBank)
  {
    return true;
  }
  else
  {
    return false;
  }
}
/******************************************************************************/
void CEEPROM::setFlagBckUpSD()
{
  this->flagsMem|=flagBckSDMemWR;
}
/******************************************************************************/
void CEEPROM::clearFlagBckUpSD()
{
  this->flagsMem&=~flagBckSDMemWR;
}
/******************************************************************************/
bool CEEPROM::getFlagBckUpSD()
{
  if(this->flagsMem & flagBckSDMemWR)
  {
    return true;
  }
  else
  {
    return false;
  }
}
/******************************************************************************/
void CEEPROM::enableFlagWritingBckUpSD()
{
  this->flagsMem|=flagWriteBckUp;
}
/******************************************************************************/
void CEEPROM::disableFlagWritingBckUpSD()
{
  this->flagsMem&=~flagWriteBckUp;
}
/******************************************************************************/
bool CEEPROM::getFlagWritingBckUpSD()
{
  if(this->flagsMem & flagWriteBckUp)
  {
    return true;
  }
  else
  {
    return false;
  }
}
/******************************************************************************/
void CEEPROM::setNewRegMPFlag()
{
  this->flagsMem|=flagNewRegMP;
}
/******************************************************************************/
void CEEPROM::clearNewRegMPFlag()
{
  this->flagsMem&=~flagNewRegMP;
}
/******************************************************************************/
bool CEEPROM::getNewRegMPFlag()
{
 /* if(this->flagsMem & flagWriteBckUp)
  {
    return true;
  }
  else
  {
    return false;
  }*/
  return (this->flagsMem & flagNewRegMP) ? true:false;
}
/******************************************************************************/


/*void CEEPROM::restoreAlarmsfromEEPROM(CReloj *myClock)
{
  char EEPROMdata;

  unsigned int startDataAddr=0x0001;
  int x;
  char *ptrData;

  this->readAddr(logicAddrEEPROM,dataFlagEEPROMAddr,1,&EEPROMdata);  // Lee el registro de Reestablecimiento de información almacenada en EEPROM externa.

  if(EEPROMdata==dataMemID)
  {//Codigo para recuperar información de la memoria EEPROM correponientes a la cantidad de alarmas y sus valores corresondientes.
    this->readAddr(logicAddrEEPROM,startDataAddr,1,&myClock->nAlarms); // Lee la cantidad de alarmas y la almacena en el campo RAM del sistema correpondiente

    startDataAddr++;

    for(x=0;x<myClock->nAlarms;x++)
    {
       ptrData=(char*)myClock->alarms[x];
       this->readAddr(logicAddrEEPROM,startDataAddr,3,ptrData); // lee y almacena en la RAM correspondiente al sistema la información última de las alarmas.
       startDataAddr+=3;
    }

  }

}*/
/*********************************************************************/
/*void CEEPROM::saveAlarmsEEPROM(CModule *mySystem)
{
  char EEPROMdata;
  unsigned int startDataAddr=0x0001;
  int x;//,y;
  char *ptrData;

  EEPROMdata=dataMemID;
  this->writeAddr(logicAddrEEPROM,dataFlagEEPROMAddr,&EEPROMdata,1);  // Almacena en memoria EEPROM el identificador de datos almacenados
  this->writeAddr(logicAddrEEPROM,startDataAddr,&mySystem->Reloj->nAlarms, 1); // Almacena la cantidad de alarmas

  startDataAddr++;

  for(x=0;x<myClock->nAlarms;x++) // Almacena Horarios de Alarmas en memeoria
  {
     ptrData=(char*)myClock->alarms[x];
     this->writeAddr(logicAddrEEPROM,startDataAddr,ptrData,3);
     startDataAddr+=3;
  }


}
*/
/*********************************************************************/



