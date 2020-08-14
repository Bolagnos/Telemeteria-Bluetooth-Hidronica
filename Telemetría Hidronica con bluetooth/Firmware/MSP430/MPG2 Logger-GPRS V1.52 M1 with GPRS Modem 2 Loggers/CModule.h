#include "CDevice.h"
//#include "CFATDriver.h"
#include "string"

#define Num_ser_flash 0x1000
#define ID_Producto   30 // Cambiarlo a ID 30
#define ID_Version    1
//Num_ser_flash es el número de serie ASCII de 6 bytes en el formato CCDDDD
// CC:    Año de produccion
// DDDD:  Numero de serie consecutivo
/*********************************************************************/
class CModule
{
private:
//   int mx,my;
//   int IDMenu;
//   int respIDMenu;

   char IDProduct[2];     // Variable de inicio que conforma el número de serie de la telemetría
   char NVersion[2];
   char ProdYear[2];
   char SerialNumber[4];  // Variable final que conforma el número de serie de la telemetría

   char SysPass[6];       // - 5 Caracteres para el password de Administrador
   char RFC[20];          // 20 caracteres para registro RFC  (Store in Memory)
   char NSM[12];          // - Numero de serie medidor (externo) (Store in Memory)
   char Latitude[12];     // Latitude decimal format with 5 decimals and sign (Store in Memory)
   char Longitude[12];    // Longitude decimal format with 5 decimals and sign (Store in Memory)
   char ID_FM_SM [3];     // - Identificador si la telemetría está asignadad a un medidor (M) o a uns sistema de medición (QA)
   char ID_UV[7];         // - Unidad Verificadora?
public:
   char flagsAlarms;      // Banderas de habilitación para requerimiento de información al activarse las alarmas.
                          // BIT 0: Se generaó una alarma para obtener infromación del medidor
   //char SysPassBuff[5];
   //bool flagStoreLoggerGPRS;
  // char UTMZone[5];

public:

//  bool menuSelection; // Identificador para detectar cuando el sistema se encuentra dentro de cualquier menu.
                       // true = Se ecnuentra en menu, false = Se ha salido del menu.
public:

   CLogger *loggerSAT;
   CLogger *loggerGPRS;
   CGPRS *ModemGPRS;
   CMPG2 *MPG2;
   CFATDriver *m_Fat;
   CRTC_3029_I2C *Reloj;
   CLedRGB *RGBLed;
   CDoor *door;
   CButtonProg *m_btnProg;
   CInputVoltmeter *VLevelBatt;
   CEEPROM *Mem;

public:

  CModule();
  ~CModule();

public:

  void setClock(char *Buffer);
  void setClockOnRTC(int *Buffer);
  void parsingTimeZone(char *Buffer);
  void setAlarm(char NRegister, char *Buffer);
  void restoreFromEEPROM();
  void saveToEEPROM();
  char* getSerialNumber();
  void clearClientLocationVars();
  //void changeRFC(char* l_RFC);
  //void changeNSM(char* l_NSM);
  //void changeLatitude(char* l_Lat);
  //void changeLongitude(char* l_Long);
  char* getExternalSerialNoFlowMet();
  char* getLocLatitude();
  char* getLocLongitude();
  char* getRFC();
  char* getIDFMorIS();
  char* getIDUV();
  char* getSysPassword();
  void restart_TX_GPRS_BT();
  //bool isFlagStoreLoggerGPRS();
  //void clearFlagStoreLoggerGPRS();
  //void setFlagStoreLoggerGPRS();
};
/*********************************************************************/
CModule::~CModule()
{}
/*********************************************************************/
CModule::CModule()
{
  //char *strDate="01012010000000"; // DDMMYYYYhhmmss
//  mx=-1;  // Column
//  my=-1; // Row
//  IDMenu=0;

 // this->flagStoreLoggerGPRS=false;

  sprintf(IDProduct,"%2d",ID_Producto);
  sprintf(NVersion,"%02d",ID_Version);
  strncpy(ProdYear,(char*) Num_ser_flash ,6);

  this->ID_FM_SM [0]='M';
  this->ID_FM_SM [1]='\0';

  this->clearClientLocationVars();

  this->flagsAlarms=0x00;
  //respIDMenu=IDMenu;
  //menuSelection=false;
  strcpy(SysPass,"19568");

  this->Mem=new CEEPROM();

  this->ModemGPRS=new CGPRS();
  this->RGBLed=new CLedRGB();
  this->MPG2=new CMPG2();
  this->m_Fat= new CFATDriver(this->MPG2->UART);
  this->Reloj=new CRTC_3029_I2C(Mem);
  this->door=new CDoor();
  this->m_btnProg=new CButtonProg();
  this->VLevelBatt=new CInputVoltmeter();
  this->loggerSAT=new CLogger();
  this->loggerSAT->setLimitAddrMem(0x04);

  this->loggerGPRS=new CLogger();
  this->loggerGPRS->setLimitAddrMem(0x04);

  this->restoreFromEEPROM();
  this->Mem->setsBank(BIT0);
  this->MPG2->restartWRPointers(0x0000);
  this->loggerSAT->clearNData();
  this->loggerGPRS->clearNData();

  //this->setClock((char*)strDate);

}
/*********************************************************************/
//const char rootMenuMessages[3][11]={"Sistema","Modem  GSM","Salir"};                              // OK
//const char MenuSystem[3][19]={"Conf. Fecha y Hora","Password","Salir"};                           // OK
//const char MenuLogger[4][19]={"Ver  Configuracion","Conf. muestreo","Hab. / Deshab.","Salir"};    // OK
//const char MenuGSM[4][15]={"Logger GSM","Reportes  GPRS","Conexion  GPRS","Salir"};               // OK
//const char MenuGSM_Reports[3][17]={"Ver Horarios","Config. Horarios","Salir"};                    // OK
//const char MenuGSM_Connection[3][21]={"Ver Config. Conexion","Seleccionar GSM SP","Salir"};       // OK
//const char MenuGSM_VerConn[6][18]={"Proveedor GSM","APN GPRS","Usuario APN GPRS","Password APN GPRS","Conexion DNS","Puerto IP"};       // OK

//const char MenuVarsReport_Devices[7][6]={"Q,V,L"," Q,V "," Q,L ","  Q  "," V,L ","  V  ","  L  "};
//const char MenuChgDatemsg[1][15]={"Manual=1",};
//const char ServicePass[1][5]={"1518"};
/*********************************************************************/

void CModule::setClock(char *Buffer)
{
   int BufferRec[6];

   sscanf(Buffer, "%2d%2d%4d%2d%2d%2d", (int*)&BufferRec[0],(int*)&BufferRec[1],(int*)&BufferRec[2],(int*)&BufferRec[3],(int*)&BufferRec[4],(int*)&BufferRec[5]);
     //flowmeter->refSTLogg.seg=Buffer[4];
     //flowmeter->refSTLogg.min=Buffer[2];
     //flowmeter->refSTLogg.hour=Buffer[0];
   this->setClockOnRTC(BufferRec);

   //this->Reloj->setClockWLCD(this->LCD, (char)BufferRec[0], (char)BufferRec[1], BufferRec[2], (char)(char)BufferRec[3], (char)BufferRec[4], (char)BufferRec[5], false); // 1/Enero/2007 01:00:00 Estado inicial.
}
/*********************************************************************/
void CModule::setClockOnRTC(int *Buffer)
{
  this->Reloj->hexToBcdRTC(Buffer);
  this->Reloj->setHour_Date(this->Mem);

//  this->Reloj->setRTCOnIntTime(this->Mem,this->LCD,true,false);
  this->Reloj->setRTCOnIntTime(this->Mem,false);

}
/*********************************************************************/
void CModule::parsingTimeZone(char *Buffer)
{
  int Valuex;
  sscanf(Buffer, "%3d", &Valuex);
  this->Reloj->setTimeZone(Valuex);
}
/*********************************************************************/
void CModule::setAlarm(char NRegister, char *Buffer)
{
  int BufferRec[2];
  NRegister--;
  sscanf(Buffer, "%2d%2d", (int*)&BufferRec[0],(int*)&BufferRec[1]);
  this->Reloj->setAlarm(NRegister,(char)BufferRec[0], (char)BufferRec[1]);
}
/*********************************************************************/
void CModule::restoreFromEEPROM()
{
  char EEPROMdata=0x00;

  unsigned long int startDataAddr;
  int x,y;
  char *ptrData,*ptrBuffer;

  // Start: 0x10000
  this->Mem->readAddr(logicAddrEEPROM,dataFlagEEPROMAddr,0x01,1,&EEPROMdata);  // Lee el registro de Reestablecimiento de información almacenada en EEPROM externa.

  if(EEPROMdata==dataMemID)
  {//Codigo para recuperar información de la memoria EEPROM correponientes a la cantidad de alarmas y sus valores corresondientes.
    startDataAddr=dataFlagEEPROMAddr+1;
    ptrBuffer=(char*)this->MPG2->UART->Buffer_Tx;
    this->Mem->readAddr(logicAddrEEPROM,startDataAddr,0x01,127,ptrBuffer); //

    ptrData=this->SysPass;          // Recupera password de usuario + RFC + Latitude + Longitude + ID_FM_SM + ID_UV
    for(x=0;x<72;x++) // 70
      ptrData[x]=*ptrBuffer++;

    /********************************************/
    startDataAddr=dataFlagEEPROMAddr+128; // inicia nuevo bloque en meemoria EEPROM
    // Start: 0x10080
    ptrBuffer=(char*)this->MPG2->UART->Buffer_Tx;
    this->Mem->readAddr(logicAddrEEPROM,startDataAddr,0x01,128,ptrBuffer); //

    ptrData=this->loggerSAT->getEnabledLoggAddr();
    for(x=0;x<4;x++)
      ptrData[x]=*ptrBuffer++;    // // Recupera variables del Logger local (Memoria SD)

    ptrData=this->loggerGPRS->getEnabledLoggAddr();
    for(x=0;x<4;x++)
      ptrData[x]=*ptrBuffer++;    // // Recupera variables del Logger para GPRS (almacenamiento en memoria I2C)

    ptrData=(char*)&this->Reloj->nAlarms;
    for(x=0;x<3;x++)
      ptrData[x]=*ptrBuffer++;    // Recupera cantidad de alarmas para envío de reporte vía satelital con time zone y regenableOp

    for(y=0;y<NAlarms;y++) // Almacena Horarios de Alarmas en memoria
    {
      ptrData=(char*)this->Reloj->alarms[y];
      for(x=0;x<3;x++)
        ptrData[x]=*ptrBuffer++;

    }

    ptrData=(char*)this->MPG2->getNMedidores();
    ptrData[0]=*ptrBuffer++;

    for(y=0;y<NumMedidores;y++) // Almacena Horarios de Alarmas en memoria
    {
      ptrData=(char*)this->MPG2->Flowmeter[y]->getIDVariable();
      for(x=0;x<5;x++)
        ptrData[x]=*ptrBuffer++;

    }

    startDataAddr=dataFlagEEPROMAddr+256; // Lee siguiente bloque de memeoria EEPROM
    // Start: 0x10100
    ptrBuffer=(char*)this->MPG2->UART->Buffer_Tx;
    this->Mem->readAddr(logicAddrEEPROM,startDataAddr,0x01,128,ptrBuffer); //

    ptrData=this->ModemGPRS->getAPN();          // Recupera password de usuario
    for(x=0;x<119;x++)
    {
      if(x==118)
      {
        ptrData[x]=(*ptrBuffer)&BIT2;
        ptrBuffer++;
      }
      else
      {
        ptrData[x]=*ptrBuffer++;
      }
    }


  }
  else
  {
     this->saveToEEPROM();
  }

}
/*********************************************************************/
void CModule::saveToEEPROM()
{
  char EEPROMdata;
  unsigned long int startDataAddr;
  int x; //,y;
  char *ptrData;


  EEPROMdata=dataMemID;

//   x=sizeof(*this->sysPass);

  // Start: 0x10000
  this->Mem->writeAddr(logicAddrEEPROM,dataFlagEEPROMAddr,0x01,&EEPROMdata,1);  // Almacena en memoria EEPROM el identificador de datos almacenados

  startDataAddr=dataFlagEEPROMAddr+1;
  this->Mem->writeAddr(logicAddrEEPROM,startDataAddr,0x01,(char*)&this->SysPass,72); // 70 // Almacena password de usuario del sistema + RFC + Latitude + Longitude + ID_FM_SM + ID_UV
  startDataAddr+=72; // 70

  startDataAddr=dataFlagEEPROMAddr+128; // inicia nuevo bloque en meemoria EEPROM
  // Start: 0x10080

  this->Mem->writeAddr(logicAddrEEPROM,startDataAddr,0x01,this->loggerSAT->getEnabledLoggAddr(),4); // Almacena variables del Logger para GPRS
   startDataAddr+=4;

  this->Mem->writeAddr(logicAddrEEPROM,startDataAddr,0x01,this->loggerGPRS->getEnabledLoggAddr(),4); // Almacena variables del Logger para GPRS
   startDataAddr+=4;

  this->Mem->writeAddr(logicAddrEEPROM,startDataAddr,0x01,(char*)&this->Reloj->nAlarms,3); // Almacena cantidad de alarmas,time zone y regenableOp
  startDataAddr+=3;

  for(x=0;x<NAlarms;x++) // Almacena Horarios (Horas, Minutos y Segundos) de Alarmas en memoria (24 alarmas)
  {
     ptrData=(char*)this->Reloj->alarms[x];
     this->Mem->writeAddr(logicAddrEEPROM,startDataAddr,0x01,ptrData,3);
     startDataAddr+=3;
  }

   this->Mem->writeAddr(logicAddrEEPROM,startDataAddr,0x01,(char*)this->MPG2->getNMedidores(),1); // Almacena la cantidad de medidores configurados
   startDataAddr+=1;

  for(x=0;x<NumMedidores;x++) // Almacena configuración de tipo de variables de cada medidor y punteros de escritura en EEPROM de Logger satelital
  {
     this->Mem->writeAddr(logicAddrEEPROM,startDataAddr,0x01,this->MPG2->Flowmeter[x]->getIDVariable(),5); //
     startDataAddr+=5;
  }

   startDataAddr=dataFlagEEPROMAddr+256; // inicia nuevo bloque en meemoria EEPROM
   // Start: 0x10100
   this->Mem->writeAddr(logicAddrEEPROM,startDataAddr,0x01,this->ModemGPRS->getAPN(),119); // Almacena la configuración del módulo GSM (conexión a GPRS).
   startDataAddr+=119;

}
/*********************************************************************/
char* CModule::getSerialNumber() // Get Telemetry Serial Number
{
  return &this->IDProduct[0];
}
/*********************************************************************/
void CModule::clearClientLocationVars()
{
  char* p_Data;
  int x,y,j;

  for(y=0;y<5;y++)
  {
    switch(y)
    {
    case 0: p_Data = this->RFC; j=sizeof(this->RFC)-1; break;
    case 1: p_Data = this->NSM; j=sizeof(this->NSM)-1; break;
    case 2: p_Data = this->Latitude; j=sizeof(this->Latitude)-1; break;
    case 3: p_Data = this->Longitude; j=sizeof(this->Longitude)-1; break;
    case 4: p_Data = this->ID_UV; j=sizeof(this->ID_UV)-1; break;
    default:
            break;
    }

    for(x=0;x<j;x++)
    {
      p_Data[x]='-';
    }
    p_Data[x]='\0';
  }
}
/*********************************************************************/
/*void CModule::changeRFC(char* l_RFC)
{
  char* p_Data;
  int x,j;

  p_Data = this->RFC;

  j=strlen(l_RFC);

  if(j < sizeof(this->RFC))
  {
    for(x=0;x<j;x++)
    {
      p_Data[x]=l_RFC[x];
    }

    for(;x<sizeof(this->RFC);x++)
    {
      p_Data[x]='\0';
    }
  }
}*/
/*********************************************************************/
/*void CModule::changeNSM(char* l_NSM) // Changes Flowmeter Serial Number
{
  char* p_Data;
  int x,j;

  p_Data = this->NSM;

  j=strlen(l_NSM);

  if(j < sizeof(this->NSM))
  {
    for(x=0;x<j;x++)
    {
      p_Data[x]=l_NSM[x];
    }

    for(;x<sizeof(this->NSM);x++)
    {
      p_Data[x]='\0';
    }
  }
}*/
/*********************************************************************/
/*void CModule::changeLatitude(char* l_Lat)
{
  char* p_Data;
  int x,j;

  p_Data = this->Latitude;

  j=strlen(l_Lat);

  if(j < sizeof(this->Latitude))
  {
    for(x=0;x<j;x++)
    {
      p_Data[x]=l_Lat[x];
    }

    for(;x<sizeof(this->Latitude);x++)
    {
      p_Data[x]='\0';
    }
  }
}*/
/*********************************************************************/
/*void CModule::changeLongitude(char* l_Long)
{
  char* p_Data;
  int x,j;

  p_Data = this->Longitude;

  j=strlen(l_Long);

  if(j < sizeof(this->Longitude))
  {
    for(x=0;x<j;x++)
    {
      p_Data[x]=l_Long[x];
    }

    for(;x<sizeof(this->Longitude);x++)
    {
      p_Data[x]='\0';
    }
  }
}*/
/*********************************************************************/
char* CModule::getExternalSerialNoFlowMet()
{
  return &this->NSM[0];
}
/*********************************************************************/
char* CModule::getLocLatitude()
{
  return &this->Latitude[0];
}
/*********************************************************************/
char* CModule::getLocLongitude()
{
  return &this->Longitude[0];
}
/*********************************************************************/
char* CModule::getRFC()
{
  return &this->RFC[0];
}
/*********************************************************************/
char* CModule::getIDFMorIS()    // Obtención de identifocador de si está dado de alta la telemetría para un medidor o para un sistema de información.
{
  return &this->ID_FM_SM[0];
}
/*********************************************************************/
char* CModule::getIDUV()
{
  return &this->ID_UV[0];
}
/*********************************************************************/
char* CModule::getSysPassword()
{
  return &this->SysPass[0];
}
/*********************************************************************/
void CModule::restart_TX_GPRS_BT()
{
 this->ModemGPRS->UART->off_RS_232();
 this->ModemGPRS->timerReset->DisableTBCCTL0_IE();

 /*if(this->m_btnProg->isButtonPressed())
 {
    this->ModemGPRS->clearValidatedPasswordFlag();
    this->m_btnProg->clearProgFlag();
    this->ModemGPRS->BL652->resetProcedure();
 }*/
 //this->m_btnProg->clearProgFlag();
 this->MPG2->requestData=false;  // Finaliza transmisiones
 this->ModemGPRS->UART->statePort&=~0x02;
 this->ModemGPRS->counterTimeout=0;
 this->Reloj->setRTCOnIntTime(this->Mem,false);
 this->RGBLed->offAll();
 this->Reloj->startReloj();
}
/**********************************************************************************/
/*bool CModule::isFlagStoreLoggerGPRS()
{
  return this->flagStoreLoggerGPRS;
}*/
/**********************************************************************************/
/*void CModule::clearFlagStoreLoggerGPRS()
{
  this->flagStoreLoggerGPRS=false;
}*/
/**********************************************************************************/
/*void CModule::setFlagStoreLoggerGPRS()
{
  this->flagStoreLoggerGPRS=true;
}*/
/**********************************************************************************/

