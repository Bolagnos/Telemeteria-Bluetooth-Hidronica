#include "CDevice.h"
#include "string"

#define Num_ser_flash 0x1000
#define ID_Producto   27 // Cambiarlo a ID 27
#define ID_Version    2
//Num_ser_flash es el número de serie ASCII de 6 bytes en el formato CCDDDD
// CC:    Año de produccion
// DDDD:  Numero de serie consecutivo
/*********************************************************************/
class CModule
{
private:
   int mx,my;
   int IDMenu;
   int respIDMenu;
   char IDProduct[2];     // Variable de inicio que conforma el número de serie
   char NVersion[2];
   char ProdYear[2];
   char SerialNumber[4];  // Variable final que conforma el número de serie (10 bytes en total)

//   int IDMenuPass;
   char SysPass[5];
   char SysPassBuff[5];

public:
   bool menuSelection; // Identificador para detectar cuando el sistema se encuentra dentro de cualquier menu.
                       // true = Se ecnuentra en menu, false = Se ha salido del menu.
public:
//   CLogger *intLogger;
   CLogger *loggerSAT;
   //CSatQ1000 *ModemSat;
   CGPRS *ModemGPRS;
   CMPG2 *MPG2;
   //CReloj *Reloj;
   CRTC_3029_I2C *Reloj;
//   CLcd *LCD;
   CLedRGB *RGBLed;
//   CKeyBoard *KBoard;
   CDoor *door;
   CButtonProg *m_btnProg;
   CInputVoltmeter *VLevelBatt;
   CEEPROM *Mem;
public:

  CModule();
  ~CModule();

  void setMenu(char nKey);
//  void exitMenu();
  void setClock(char *Buffer);
  void setClockOnRTC(int *Buffer);
  void parsingTimeZone(char *Buffer);
//  void printLCDDate();
  void setAlarm(char NRegister, char *Buffer);

  void restoreFromEEPROM();
  void saveToEEPROM();

//  bool validateHourKB(char *Buffer);
  void clearBufferKB();
//  bool displayShedule(char noReg);
//  bool displayConfDevice(char noReg);

  char* getSerialNumber();
//  void enterPasswordLCDPromt();

//  bool validateAdressMB_KB(char *Buffer);
//  void displayGSMVar(int nVar);
};
/*********************************************************************/
CModule::~CModule()
{}
/*********************************************************************/
CModule::CModule()
{
  mx=-1;  // Column
  my=-1; // Row
  IDMenu=0;

  sprintf(IDProduct,"%2d",ID_Producto);
  sprintf(NVersion,"%02d",ID_Version);
  strncpy(ProdYear,(char*) Num_ser_flash ,6);
  respIDMenu=IDMenu;
  menuSelection=false;
  strcpy(SysPass,"1956");

  this->Mem=new CEEPROM();

  //this->ModemSat=new CSatQ1000();
  this->ModemGPRS=new CGPRS();
//  this->LCD=new CLcd();
  this->RGBLed=new CLedRGB();
  this->MPG2=new CMPG2();
 // Reloj=new CReloj();
  this->Reloj=new CRTC_3029_I2C(Mem);
//  this->KBoard= new CKeyBoard();
//  this->intLogger=new CLogger();
  this->door=new CDoor();
  this->m_btnProg=new CButtonProg();
  this->VLevelBatt=new CInputVoltmeter();
  this->loggerSAT=new CLogger();

//  this->intLogger->setLimitAddrMem(0x03);
  this->loggerSAT->setLimitAddrMem(0x04);
//  this->intLogger->setPointerWrite(0x000000); // Puntero de inicio de memoria del Logger interno
//  this->loggerSAT->setPointerWrite(0x060000); // Puntero de inicio de memoria para datos satelitales
//  this->intLogger->setDefSampleTime(20);      // Establece tiempo de muestreo en segundos por default 20 seg
//////  this->loggerSAT->setDefSampleTime(&this->KBoard->BufferKB[1]);    // 1800 Establece tiempo de muestreo en segundos por default 1800 seg (30 min)

//  this->ModemSat->disableSendingReport();
  this->restoreFromEEPROM();
  this->MPG2->restartWRPointers();
  this->loggerSAT->clearNData();

}
/*********************************************************************/
const char rootMenuMessages[3][11]={"Sistema","Modem  GSM","Salir"};                              // OK
const char MenuSystem[3][19]={"Conf. Fecha y Hora","Password","Salir"};                           // OK
const char MenuLogger[4][19]={"Ver  Configuracion","Conf. muestreo","Hab. / Deshab.","Salir"};    // OK
const char MenuGSM[4][15]={"Logger GSM","Reportes  GPRS","Conexion  GPRS","Salir"};               // OK
const char MenuGSM_Reports[3][17]={"Ver Horarios","Config. Horarios","Salir"};                    // OK
const char MenuGSM_Connection[3][21]={"Ver Config. Conexion","Seleccionar GSM SP","Salir"};       // OK
const char MenuGSM_VerConn[6][18]={"Proveedor GSM","APN GPRS","Usuario APN GPRS","Password APN GPRS","Conexion DNS","Puerto IP"};       // OK

//const char MenuVarsReport_Devices[7][6]={"Q,V,L"," Q,V "," Q,L ","  Q  "," V,L ","  V  ","  L  "};
const char MenuChgDatemsg[1][15]={"Manual=1",};
const char ServicePass[1][5]={"1518"};

void CModule::setMenu(char nKey)
{
/*
   int myLen;
   char* p_Msg;

   switch(IDMenu)
   {
   case 0: switch(nKey) // Menu principal
          {
          case '*':   // Selección visual del menú principal
                    menuSelection=true;
                    mx++;
                    this->LCD->configLCD();
                    switch(mx)
                    {
                    case 0: my=0x86; break; // "my", indica la posición del primer mensaje en LCD en la primera línea.
                    case 1: my=0x85; break;
                    case 2:
                    default:
                            my=0x87;
                            myLen=strlen(rootMenuMessages[mx]);
                            this->LCD->sendMessage(my,myLen,(char*)rootMenuMessages[mx],0xC1,18,"Sel:*      Enter:#", true);
                            mx=-1;
                            break;
                    }
                    if(mx>-1)
                    {
                      myLen=strlen(rootMenuMessages[mx]);
                      this->LCD->sendMessage(my,myLen,(char*)rootMenuMessages[mx],0xC1,18,"Sel:*      Enter:#", true);
                    }
                    break;
          case '#': // Enter en el menu principal
                    switch(mx)
                    {
                    case 0: IDMenu=1; mx=0;
                            p_Msg=(char*)MenuSystem[mx];
                            myLen=strlen(p_Msg);
                            my=0x81;
                            break;
                    case 1:
                            IDMenu=3; mx=0;
                            p_Msg=(char*)MenuGSM[mx];
                            myLen=strlen(p_Msg);
                            my=0x85;
                            break;
                    case -1:
                    default: this->LCD->configLCD(); this->exitMenu(); break;
                    }
                    if(mx>-1)
                    {
                      this->LCD->sendMessage(my,myLen,p_Msg,0xC1,18,"Sel:*      Enter:#", true);
                    }
                    break;
          default: this->LCD->configLCD();  break;
          }break; // FIN CASE IDMENU=0
///////////////////////////////////////////////////////////////////////////////////////////////////
   case 1: switch(nKey) // Menu de System
          {
          case '*':   // Selección visual del menú principal
                    menuSelection=true;
                    mx++;
                    this->LCD->configLCD();
                    switch(mx)
                    {
                    case 0: my=0x81; break; // "my", indica la posición del primer mensaje en LCD en la primera línea.
                    case 1: my=0x86; break;
                    case 2:
                    default:
                            my=0x87;
                            myLen=strlen(MenuSystem[mx]);
                            this->LCD->sendMessage(my,myLen,(char*)MenuSystem[mx],0xC1,18,"Sel:*      Enter:#", true);
                            mx=-1;
                            break;
                    }
                    if(mx>-1)
                    {
                      myLen=strlen(MenuSystem[mx]);
                      this->LCD->sendMessage(my,myLen,(char*)MenuSystem[mx],0xC1,18,"Sel:*      Enter:#", true);
                    }
                    break;
          case '#': // Enter en el menu de System
                    switch(mx)
                    {
                    case 0: IDMenu=11; mx=0; respIDMenu=IDMenu;  // Cambio de fecha y hora
                            IDMenu=10;
                            this->enterPasswordLCDPromt();
                            break;
                    case 1: IDMenu=12; mx=0; respIDMenu=IDMenu;  // Cambio de password
                            IDMenu=10;
                            this->enterPasswordLCDPromt();
                            break;
                    case -1:
                    default: this->LCD->configLCD(); this->exitMenu(); break;
                    }
                    break;
          default: this->LCD->configLCD();  break; //this->printLCDDate(); break;
          }break; // FIN CASE IDMENU=1
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
   case 3: switch(nKey) // Menu de Modem GSM
          {
          case '*':   // Selección visual del menú Modem GSM
                    menuSelection=true;
                    mx++;
                    this->LCD->configLCD();
                    switch(mx)
                    {
                    case 0: my=0x85; break; // "my", indica la posición del primer mensaje en LCD en la primera línea.
                    case 2:
                    case 1: my=0x83; break;
                    case 3:
                    default:
                            my=0x87;
                            myLen=strlen(MenuGSM[mx]);
                            this->LCD->sendMessage(my,myLen,(char*)MenuGSM[mx],0xC1,18,"Sel:*      Enter:#", true);
                            mx=-1;
                            break;
                    }
                    if(mx>-1)
                    {
                      myLen=strlen(MenuGSM[mx]);
                      this->LCD->sendMessage(my,myLen,(char*)MenuGSM[mx],0xC1,18,"Sel:*      Enter:#", true);
                    }
                    break;
          case '#': // Enter en el menu de Modem GSM
                    switch(mx)
                    {
                    case 0: IDMenu=30; mx=0;              // Logger GSM
                            p_Msg=(char*)MenuLogger[mx];
                            myLen=strlen(p_Msg);
                            my=0x81;
                            break;
                    case 1: IDMenu=31; mx=0;              // Reportes GPRS
                            p_Msg=(char*)MenuGSM_Reports[mx];
                            myLen=strlen(p_Msg);
                            my=0x84;
                            break;
                    case 2: IDMenu=32; mx=0;              // Conexion GPRS
                            p_Msg=(char*)MenuGSM_Connection[mx];
                            myLen=strlen(p_Msg);
                            my=0x80;
                            break;
                    case -1:
                    default: this->LCD->configLCD(); this->exitMenu(); break;
                    }
                    if(mx>-1)
                    {
                      this->LCD->sendMessage(my,myLen,p_Msg,0xC1,18,"Sel:*      Enter:#", true);
                    }

                    break;
          default: this->LCD->configLCD();  break;
          }break; // FIN CASE IDMENU=3

///////////////////////////////////////////////////////////////////////////////////////////////////
   case 30: switch(nKey) // Menu de Modem GSM->Logger
          {
          case '*':   // Selección visual del menú Logger
                    menuSelection=true;
                    mx++;
                    this->LCD->configLCD();
                    switch(mx)
                    {
                    case 0: my=0x81; break; // "my", indica la posición del primer mensaje en LCD en la primera línea.
                    case 1:
                    case 2: my=0x83; break;
                    case 3:
                    default:
                            my=0x87;
                            myLen=strlen(MenuLogger[mx]);
                            this->LCD->sendMessage(my,myLen,(char*)MenuLogger[mx],0xC1,18,"Sel:*      Enter:#", true);
                            mx=-1;
                            break;
                    }
                    if(mx>-1)
                    {
                      myLen=strlen(MenuLogger[mx]);
                      this->LCD->sendMessage(my,myLen,(char*)MenuLogger[mx],0xC1,18,"Sel:*      Enter:#", true);
                    }
                    break;
          case '#': // Enter en el menu de Modem Satelital->Logger
                    switch(mx)
                    {
                    case 0: IDMenu=21;         // Vista de tiempos de muestreo y estado del logger GSM
                    this->LCD->sendMessage(0x80,20,"Tiempo DLS: 00:00:00",0xC0,20,"Estado:     Oprima:#", true);
//////                            *(unsigned long int*)&this->KBoard->BufferKB[1]=*this->loggerSAT->getDefSampleTime();
                            mx=*this->loggerSAT->getIndexSampleTime();
                            switch(mx)
                            {
                            case 2:         // Para minutos
                            case 1:
                                    my=0x8F;
                                    break;
                            case 0:         // En caso que sea 0
                                    my=0x92;
                                     break;
                            default:        // Para horas
                                    my=0x8C;
                                    break;
                            }
                            mx=*this->loggerSAT->getTimeLoggerSatByIndex(mx);
                            sprintf((char*)&this->KBoard->BufferKB[0],"%0.2d",mx);
                            if(this->loggerSAT->isEnabledLogg())
                            {this->KBoard->BufferKB[2]='E';}
                            else
                            {this->KBoard->BufferKB[2]='D';}
                            this->LCD->sendMessage(my,2,(char*)&this->KBoard->BufferKB[0],0xC8,1,(char*)&this->KBoard->BufferKB[2],false);
                            my=0;
                            break;
                    case 1: IDMenu=302; mx=0; respIDMenu=IDMenu;  // Cambio de tiempo de recolección logger GSM
                            IDMenu=10;
                            this->enterPasswordLCDPromt();
                            break;
                    case 2: IDMenu=303; mx=0; respIDMenu=IDMenu;  // Habilitación o deshabilitación de logger para reporte satelital
                            IDMenu=10;
                            this->enterPasswordLCDPromt();
                            break;
                    case -1:
                    default: this->LCD->configLCD(); this->exitMenu(); break;
                    }

                    break;
          default: this->LCD->configLCD();  break; //this->printLCDDate(); break;
          }break; // FIN CASE IDMENU=30

///////////////////////////////////////////////////////////////////////////////////////////////////
   case 31: switch(nKey) // Menu de Modem GSM->Reporte
          {
          case '*':   // Selección visual del menú principal
                    menuSelection=true;
                    mx++;
                    this->LCD->configLCD();
                    switch(mx)
                    {
                    case 0: my=0x84; break; // "my", indica la posición del primer mensaje en LCD en la primera línea.
                    case 1: my=0x82; break;
                    case 2:
                    default:
                            my=0x87;
                            myLen=strlen(MenuGSM_Reports[mx]);
                            this->LCD->sendMessage(my,myLen,(char*)MenuGSM_Reports[mx],0xC1,18,"Sel:*      Enter:#", true);
                            mx=-1;
                            break;
                    }
                    if(mx>-1)
                    {
                      myLen=strlen(MenuGSM_Reports[mx]);
                      this->LCD->sendMessage(my,myLen,(char*)MenuGSM_Reports[mx],0xC1,18,"Sel:*      Enter:#", true);
                    }
                    break;
          case '#': // Enter en el menu de Modem GSM->Reporte
                    switch(mx)
                    {
                    case 0: IDMenu=311; my=0;         // Vista de horarios configurados
                            if(this->displayShedule(my))
                            {my++;}
                            else
                            {this->LCD->sendMessage(0x80,19,"No existen horarios",0xC4,12,"Configurados", true);
                             my=4;
                            }
                            break;
                    case 1: IDMenu=312; mx=0; respIDMenu=IDMenu;  // Configurar Horarios de comunicación Satelital
                            IDMenu=10;
                            this->enterPasswordLCDPromt();
                            break;
                    case -1:
                    default: this->LCD->configLCD(); this->exitMenu(); break;
                    }

                    break;
          default: this->LCD->configLCD();  break; //this->printLCDDate(); break;
          }break; // FIN CASE IDMENU=31
///////////////////////////////////////////////////////////////////////////////////////////////////
   case 32: switch(nKey) // Menu de Modem GSM->Conexion
          {
          case '*':   // Selección visual del menú principal
                    menuSelection=true;
                    mx++;
                    this->LCD->configLCD();
                    switch(mx)
                    {
                    case 0: my=0x80; break; // "my", indica la posición del primer mensaje en LCD en la primera línea.
                    case 1: my=0x81; break;
                    case 2:
                    default:
                            my=0x87;
                            myLen=strlen(MenuGSM_Connection[mx]);
                            this->LCD->sendMessage(my,myLen,(char*)MenuGSM_Connection[mx],0xC1,18,"Sel:*      Enter:#", true);
                            mx=-1;
                            break;
                    }
                    if(mx>-1)
                    {
                      myLen=strlen(MenuGSM_Connection[mx]);
                      this->LCD->sendMessage(my,myLen,(char*)MenuGSM_Connection[mx],0xC1,18,"Sel:*      Enter:#", true);
                    }
                    break;
          case '#': // Enter en el menu de Modem GSM->Conexion
                    switch(mx)
                    {
                    case 0: IDMenu=321; my=0; respIDMenu=IDMenu;  // Vista de conf. de Conexión GPRS
                            IDMenu=10;
                            this->enterPasswordLCDPromt();
                            break;
                    case 1: IDMenu=322; mx=0; respIDMenu=IDMenu;  // Configurar Variables de Conexión a servicio GPRS
                            IDMenu=10;
                            this->enterPasswordLCDPromt();
                            break;
                    case -1:
                    default: this->LCD->configLCD(); this->exitMenu(); break;
                    }

                    break;
          default: this->LCD->configLCD();  break; //this->printLCDDate(); break;
          }break; // FIN CASE IDMENU=32

///////////////////////////////////////////////////////////////////////////////////////////////////
  case 10:  // Ingreso y evaluación de password

          switch(nKey)
          {
          case '#':
                  if(this->KBoard->lenDataBuf==4)
                  {
                    this->KBoard->BufferKB[4]='\0';

                    strcpy((char*)&this->SysPassBuff[0],(char*)&ServicePass[0]);
                    this->SysPassBuff[0]--;
                    this->SysPassBuff[1]++;
                    this->SysPassBuff[2]--;
                    this->SysPassBuff[3]++;

                    if(!(strncmp(&this->KBoard->BufferKB[0],(char*)&this->SysPass,4)) || !(strncmp(&this->KBoard->BufferKB[0],(char*)this->SysPassBuff,4)))
                    {
                                            // Envía a menú correpondiente a variables a modificar
                      switch(respIDMenu)
                      {
                      case 11: IDMenu=11; mx=0;
                            myLen=strlen(MenuChgDatemsg[mx]);
                            this->LCD->sendMessage(0x86,myLen,(char*)MenuChgDatemsg[mx],0xC5,10,"Cancelar=2", true);
                            this->LCD->exitLCDEdit();
                            break;
                      case 12: IDMenu=12; mx=0; my=0;
                              this->KBoard->BufferKB[0]=mx;
                              this->KBoard->lenDataBuf=0;
                              this->LCD->sendMessage(0x82,16,"Nuevo Password ?",0xC8,4,"----", true); // menu para actualización de alarmas
                              this->LCD->setAddr(0xC8);
                              this->LCD->setCaracter('-');
                              this->LCD->configLCDEdit();
                              this->LCD->setAddr(0xC8);
                              break;
                      case 302: IDMenu=302; my=1;   // Para seleccionar entre 15, 30 y 60 min
                              switch(*this->MPG2->getNMedidores())
                              {
                              case 4: my=3; mx=2; break;  // Para seleccionar entre 60 min
                              case 3:
                              case 2: my=2;               // Para seleccionar entre 30 y 60 min
                              default: mx=1; break;
                              }
//                              *(unsigned long int*)&this->KBoard->BufferKB[1]=(unsigned long int)*this->loggerSAT->getTimeLoggerSatByIndex(my);
                              this->LCD->sendMessage(0x80,20,"Tiempo:       #:Ent.",0xC1,18,"*:Sel.  4:Cancelar", true); // menu para actualización de alarmas
                              sprintf((char*)&this->KBoard->BufferKB[0],"%2d %s",*this->loggerSAT->getTimeLoggerSatByIndex(my),this->loggerSAT->getTimeUnitsByIndex(mx));
                              this->LCD->sendMessage(0x87,6,&this->KBoard->BufferKB[0],0xC1,0,"", false); // menu para actualización de alarmas
                              this->LCD->configLCDEdit();
                              this->LCD->setAddr(0x88);
                              mx=0;
                              break;
                      case 303: IDMenu=303; mx=0;
                              this->LCD->exitLCDEdit();
                              this->LCD->sendMessage(0x83,17,"Habilitar? #:Ent.",0xC1,18,"*:Sel.  4:Cancelar", true); // menu para actualización de alarmas
                              break;
                      case 312: IDMenu=312; mx=0;
                              this->KBoard->BufferKB[0]=mx;
                              this->LCD->sendMessage(0x80,20,"Horarios:     #:Ent.",0xC1,18,"*:Sel.  4:Cancelar", true); // menu para actualización de alarmas
                              this->LCD->setAddr(0x8B);
                              this->LCD->setCaracter(0x30);
                              this->LCD->configLCDEdit();
                              this->LCD->setAddr(0x8B);
                              break;
                      case 321: IDMenu=321; my=0; mx=0;
                              this->LCD->exitLCDEdit();
                              this->displayGSMVar(my);
                              break;
                      case 322: IDMenu=322; mx=0;
                              this->LCD->exitLCDEdit();
                              this->LCD->sendMessage(0x81,18,"Cambiar Prov. GSM?",0xC1,18,"#:Ent.  4:Cancelar", true); // menu para actualización de alarmas
                              break;

                      default: this->exitMenu();

                      }

                    }
                    else
                    {
                      respIDMenu=IDMenu;
                      this->LCD->exitLCDEdit();
                      this->LCD->sendMessage(0x87,5,"ERROR",0xC0,20,"Password  Incorrecto", true); // menu para actualización de alarmas
                      IDMenu=255;
                    }
                  }
                  break;
          case '*': //IDMenu=0; mx=-1; this->setMenu('#');// Sale de todo el menu
                    // Regreso del cursor
                    this->KBoard->lenDataBuf=0;

                    this->LCD->setAddr(0xC8);
                    this->LCD->setCaracter(0x20);
                    this->LCD->setAddr(0xD1);
                    this->LCD->setCaracter(0x20);
                    this->LCD->setCaracter(0x20);
                    this->LCD->setCaracter(0x20);
                    this->LCD->sendMessage(0xC8,4,"----",0xC9,0,"", false); // menu para actualización de alarmas
                    this->LCD->setAddr(0xC8);

                    break;
          default:  // 1,2,3,4,5,6,7,8,9,0
                      this->KBoard->BufferKB[this->KBoard->lenDataBuf]=nKey;
                      this->LCD->setCaracter('*');
                      this->KBoard->lenDataBuf++;

                      if(this->KBoard->lenDataBuf>=4)
                      {
                        this->KBoard->lenDataBuf=4;
                        this->LCD->setAddr(0x93);
                        this->LCD->setCaracter('#');
                        this->LCD->setAddr(0xD1);
                        this->LCD->setCaracter('O');
                        this->LCD->setCaracter('K');
                        this->LCD->setCaracter('?');
                      }
                    break;
          }
          break; // Fin de  ingreso y evaluación de Password
///////////////////////////////////////////////////////////////////////////////////////////////////
   case 255:  // Ejecuciones cuando existe condición de error
          switch(nKey)
          {
          case '#':
                  switch(respIDMenu)
                  {
                  case 3121: IDMenu=312; mx=this->Reloj->getNAlarms(); break;
                  case 3221: IDMenu=322; mx=*this->MPG2->getNMedidores(); break;
                  case 10:
                  case 12:
                          this->exitMenu(); break;
                  default: break;
                  }
                  this->setMenu('#');
                  break;

          default: break;
          }
          break;
///////////////////////////////////////////////////////////////////////////////////////////////////
   case 11:
          switch(nKey)  // muestra menu de selección del modo de actualización de fecha y hora
          {
          case '1': IDMenu=111; this->KBoard->lenDataBuf=0;
                    this->LCD->sendMessage(0x84,12,"Zona horaria",0xC3,14,"1:+  2:-  + HH", true);
                    this->KBoard->BufferKB[0]='+';
                    this->LCD->configLCDEdit();
                    this->LCD->setAddr(0xCD);
                    break; // Insertar rutina para cambiar la fcha y hora por medio del GPS
          case '2': this->exitMenu(); break; // Sale de todo el menu
          }break;

   ////////////
    case 111:
          switch(nKey)  // muestra menu de selección de Time Zone
          {
          case '#':
                  if(this->KBoard->lenDataBuf<3)
                  {
                    this->LCD->shiftLCDRight();
                    this->KBoard->lenDataBuf++;

                    if(this->KBoard->lenDataBuf==1)
                    {
                      this->LCD->sendMessage(0x80,0,"",0xC3,8,"(00-12) ", false);
                      this->LCD->setAddr(0xCF);
                    }
                    if(this->KBoard->lenDataBuf==3)
                    {
                      this->LCD->setAddr(0x93);
                      this->LCD->setCaracter('#');
                      this->LCD->setAddr(0xD2);
                      this->LCD->setCaracter('O');
                      this->LCD->setCaracter('K');
                    }
                  }
                  else
                  {// Establece los valores selecconados por teclado en el registro del reloj inmerso.
                    IDMenu=1111;
                    //this->exitMenu(); // Sale de todo el menu
                    this->parsingTimeZone(this->KBoard->BufferKB);

                    this->saveToEEPROM();
                    this->clearBufferKB();
                    this->KBoard->lenDataBuf=0;

                    this->LCD->sendMessage(0x81,17,"Fecha: dd/mm/yyyy",0xC2,15,"Hora:  hh:mm:ss", true);
                    this->LCD->configLCDEdit();
                    this->LCD->setAddr(0x88);
                    //this->ModemSat->getTimeDateSat201C();
                  }
                  break;

          case '*': // Regreso de cursor
                      this->KBoard->lenDataBuf--;
                      if(this->KBoard->lenDataBuf<0)
                      {
                         this->KBoard->lenDataBuf=0;
                      }
                      else
                      {
                        this->LCD->shiftLCDLeft();
                        if(this->KBoard->lenDataBuf==0)
                        {
                          this->LCD->sendMessage(0x80,0,"",0xC3,8,"1:+  2:-", false);
                          this->LCD->setAddr(0xCD);
                        }
                        if(this->KBoard->lenDataBuf==2)
                        {
                        this->LCD->setAddr(0x93);
                        this->LCD->setCaracter(0x20);
                        this->LCD->setAddr(0xD2);
                        this->LCD->setCaracter(0x20);
                        this->LCD->setCaracter(0x20);
                        this->LCD->setAddr(0xD0);
                        }
                      }
                      break;
          default:  // 1,2,3,4,5,6,7,8,9,0
                  if(this->KBoard->lenDataBuf<3 && this->KBoard->lenDataBuf>0)
                  {
                    this->KBoard->BufferKB[this->KBoard->lenDataBuf]=nKey;
                    this->LCD->setCaracter(nKey);
                    this->KBoard->lenDataBuf++;

                    if(this->KBoard->lenDataBuf==3)
                    {
                      this->LCD->setAddr(0x93);
                      this->LCD->setCaracter('#');
                      this->LCD->setAddr(0xD2);
                      this->LCD->setCaracter('O');
                      this->LCD->setCaracter('K');
                    }
                  }
                  else
                  {
                    switch(nKey)
                    {
                    case '1': nKey=0x2B; //Caracter +
                              break;
                    case '2': nKey=0x2D; //Caracter -
                    default: break;
                    }

                    if(nKey==0x2B || nKey==0x2D)
                    {
                      this->KBoard->BufferKB[this->KBoard->lenDataBuf]=nKey;
                      this->LCD->setCaracter(nKey);
                      this->LCD->sendMessage(0x80,0,"",0xC3,8,"(00-12) ", false);
                      this->KBoard->lenDataBuf++;
                      this->LCD->setAddr(0xCF);
                    }
                  }
                  break;
          }break;
///////////////////////////////////////////////////////////////////////////////////////////////////
   case 1111:
          switch(nKey)  // Muestra menu de selección del modo de actualización de fecha y hora
          {
          case '#':
                  if(this->KBoard->lenDataBuf<14)
                  {
                    this->LCD->shiftLCDRight();
                    this->KBoard->lenDataBuf++;
                    if(this->KBoard->lenDataBuf==2)
                    {
                      this->LCD->setAddr(0x8B);
                    }
                    if(this->KBoard->lenDataBuf==4)
                    {
                      this->LCD->setAddr(0x8E);
                    }
                    if(this->KBoard->lenDataBuf==8)
                    {
                      this->LCD->setAddr(0xC9);
                    }
                    if(this->KBoard->lenDataBuf==10)
                    {
                      this->LCD->setAddr(0xCC);
                    }
                    if(this->KBoard->lenDataBuf==12)
                    {
                      this->LCD->setAddr(0xCF);
                    }
                    if(this->KBoard->lenDataBuf==14)
                    {
                      this->LCD->setAddr(0x93);
                      this->LCD->setCaracter('#');
                      this->LCD->setAddr(0xD2);
                      this->LCD->setCaracter('O');
                      this->LCD->setCaracter('K');
                    }
                  }
                  else
                  {// Establece los valores selecconados por teclado en el registro del reloj inmerso.

                     this->setClock(this->KBoard->BufferKB);
                     //this->Reloj->startRTCCLK(this->Mem);
                     this->exitMenu(); // Sale de todo el menu
                  }
                  break;

          case '*': // Regreso de cursor
                      this->KBoard->lenDataBuf--;
                      if(this->KBoard->lenDataBuf<0)
                      {
                         this->KBoard->lenDataBuf=0;
                      }
                      else
                      {
                        this->LCD->shiftLCDLeft();
                        if(this->KBoard->lenDataBuf==3)
                        {
                          this->LCD->setAddr(0x8C);
                        }
                        if(this->KBoard->lenDataBuf==1)
                        {
                        this->LCD->setAddr(0x89);
                        }

                        if(this->KBoard->lenDataBuf==7)
                        {
                          this->LCD->setAddr(0x91);
                        }
                        if(this->KBoard->lenDataBuf==9)
                        {
                        this->LCD->setAddr(0xCA);
                        }
                        if(this->KBoard->lenDataBuf==11)
                        {
                        this->LCD->setAddr(0xCD);
                        }
                        if(this->KBoard->lenDataBuf==13)
                        {
                        this->LCD->setAddr(0x93);
                        this->LCD->setCaracter(0x20);
                        this->LCD->setAddr(0xD2);
                        this->LCD->setCaracter(0x20);
                        this->LCD->setCaracter(0x20);
                        this->LCD->setAddr(0xD0);
                        }
                      }
                      break;
          default: //1,2,3,4,5,6,7,8,9,0
                  if(this->KBoard->lenDataBuf<14)
                  {
                    this->KBoard->BufferKB[this->KBoard->lenDataBuf]=nKey;
                    this->LCD->setCaracter(nKey);
                    this->KBoard->lenDataBuf++;
                    if(this->KBoard->lenDataBuf==2)
                    {
                      this->LCD->setAddr(0x8B);
                    }
                    if(this->KBoard->lenDataBuf==4)
                    {
                      this->LCD->setAddr(0x8E);
                    }
                    if(this->KBoard->lenDataBuf==8)
                    {
                      this->LCD->setAddr(0xC9);
                    }
                    if(this->KBoard->lenDataBuf==10)
                    {
                      this->LCD->setAddr(0xCC);
                    }
                    if(this->KBoard->lenDataBuf==12)
                    {
                      this->LCD->setAddr(0xCF);
                    }
                    if(this->KBoard->lenDataBuf==14)
                    {
                      this->LCD->setAddr(0x93);
                      this->LCD->setCaracter('#');
                      this->LCD->setAddr(0xD2);
                      this->LCD->setCaracter('O');
                      this->LCD->setCaracter('K');
                    }
                  }

                  break;
          }break;
///////////////////////////////////////////////////////////////////////////////////////////////////
   case 12:
          switch(nKey)
          {
          case '#':
                  if(this->KBoard->lenDataBuf==4)
                  {
                    this->KBoard->BufferKB[4]='\0';
                    this->SysPassBuff[4]='\0';
                    my++;
                    if(my==1)
                    {
                      this->KBoard->lenDataBuf=0;
                      this->LCD->sendMessage(0x81,18,"Reingrese Password",0xC8,4,"----", true); // menu para actualización de alarmas
                      this->LCD->setAddr(0xC8);
                      this->LCD->setCaracter('-');
                      this->LCD->configLCDEdit();
                      this->LCD->setAddr(0xC8);
                    }
                    else
                    {
                      if(strncmp(&this->KBoard->BufferKB[0],(char*)&this->SysPassBuff,4))
                      {
                        respIDMenu=IDMenu;
                        this->LCD->exitLCDEdit();
                        this->LCD->sendMessage(0x82,16,"ERROR: Passwords",0xC3,14,"NO  Concuerdan", true); // menu para actualización de alarmas
                        IDMenu=255;
                      }
                      else
                      {
                        strcpy(this->SysPass,this->KBoard->BufferKB);
                        this->saveToEEPROM();
                        this->exitMenu();

                      }
                    }

                  }
                  break;
          case '*': //IDMenu=0; mx=-1; this->setMenu('#');// Sale de todo el menu
                    // Regreso del cursor
                    this->KBoard->lenDataBuf=0;

                    this->LCD->setAddr(0xC8);
                    this->LCD->setCaracter(0x20);
                    this->LCD->setAddr(0xD1);
                    this->LCD->setCaracter(0x20);
                    this->LCD->setCaracter(0x20);
                    this->LCD->setCaracter(0x20);
                    this->LCD->sendMessage(0xC8,4,"----",0xC9,0,"", false); // menu para actualización de alarmas
                    this->LCD->setAddr(0xC8);

                    break;
          default:  // 1,2,3,4,5,6,7,8,9,0
                      if(my==0)
                        this->KBoard->BufferKB[this->KBoard->lenDataBuf]=nKey;
                      else
                        this->SysPassBuff[this->KBoard->lenDataBuf]=nKey;

                      this->LCD->setCaracter('*');
                      this->KBoard->lenDataBuf++;

                      if(this->KBoard->lenDataBuf>=4)
                      {
                        this->KBoard->lenDataBuf=4;
                        this->LCD->setAddr(0x93);
                        this->LCD->setCaracter('#');
                        this->LCD->setAddr(0xD1);
                        this->LCD->setCaracter('O');
                        this->LCD->setCaracter('K');
                        this->LCD->setCaracter('?');
                      }
                    break;
          }
          break;
////////////////
    case 311:
          switch(nKey)
          {
          case'#':
                  if(this->displayShedule(my))
                  {my++;}

                  else
                  {
                    if(my<this->Reloj->getNAlarms()+1)
                    {
                       this->LCD->sendMessage(0x81,18,"Salir Ver Horarios",0xC4,12,"Configurados", true);
                       my=this->Reloj->getNAlarms()+1;
                    }
                    else
                    {
                      my=0; this->exitMenu();
                    }
                  }

                  break;
          case'*':
                   my-=2;
                   if(my<=-1)
                   {
                     my=0;
                   }
                  this->setMenu('#');
                  break;

          default: break;
          }
          break;
////////////////
    case 321:
          switch(nKey)
          {
          case'#':
                  my++;
                  if(my<6)
                  {
                    this->displayGSMVar(my);
                  }

                  else
                  {
                    if(my<7)
                    {
                       this->LCD->sendMessage(0x82,15,"Salir Ver Conf.",0xC3,14,"Conexion  GPRS", true);
                       my=6;
                    }
                    else
                    {
                      my=0; this->exitMenu();
                    }
                  }

                  break;
          case'*':
                  my-=2;
                  if(my<-1)
                  {
                    my=-1;
                  }
                  this->setMenu('#');
                  break;

          default: break;
          }
          break;
////////////////
   case 312:
          switch(nKey)
          {
          case '*': // Select
                    mx++;
                    if(mx==25)
                    {mx=0;}

                    if(mx<=9)
                    {
                      this->LCD->setAddr(0x8A);
                      this->LCD->setCaracter(0x20);
                      this->LCD->setAddr(0x8B);
                      this->LCD->setCaracter(0x30+mx);
                    }
                    else
                    {
                      sprintf(&this->KBoard->BufferKB[0],"%2d",mx);
                      this->LCD->setAddr(0x8A);
                      this->LCD->setCaracter(this->KBoard->BufferKB[0]);
                      this->LCD->setAddr(0x8B);
                      this->LCD->setCaracter(this->KBoard->BufferKB[1]);
                    }
                    this->LCD->shiftLCDLeft();
//                    this->KBoard->BufferKB[0]=mx;
                    break;
          case '#':  // Enter
                    this->Reloj->setNAlarms(mx);
                    if(mx)
                    {
                      IDMenu=3121;
                      my=1;
                      respIDMenu=IDMenu;
                      this->clearBufferKB();
                      this->LCD->sendMessage(0x83,10,"Horario #:",0xC3,13,"HH:MM (00-23)", true); // menu para actualización de alarmas
                      this->LCD->setAddr(0x8F);
                      this->LCD->setCaracter(0x30+my);
                      this->LCD->configLCDEdit();
                      this->LCD->setAddr(0xC3);
                      this->KBoard->lenDataBuf=0;
                    }
                    else
                    {
                      this->saveToEEPROM();
                      this->exitMenu(); // Sale de todo el menu
                      //mx=-1; IDMenu=0; this->LCD->exitLCDEdit(); menuSelection=false; this->printLCDDate(); // Sale de todo el menu
                    }
                    break;

          case '4': this->exitMenu(); //mx=-1; IDMenu=0; this->LCD->exitLCDEdit(); menuSelection=false; this->printLCDDate();// Sale de todo el menu
                    break;

          }
          break;
////////////////
   case 322:
          switch(nKey)
          {
          case '#':  // Enter
                    IDMenu=3221; mx=0; my=0;
                    this->LCD->sendMessage(0x80,12,"Proveedor:          ",0xC0,20,"*:Sel. #:Ent. 4:Sal.", true); // menu para actualización de proveedor de servicio GSM
                    this->LCD->sendMessage(0x8B,6,this->ModemGPRS->getAPNPorvByIndexList(my),0xC0,0,"",false);
                    break;

          case '4': this->exitMenu();
                    break;

          }
          break;
////////////////////////////////////
   case 3121:

          switch(nKey)
          {
          case '#':
                  if(this->KBoard->lenDataBuf<4)
                  {
                    this->LCD->shiftLCDRight();
                    this->KBoard->lenDataBuf++;

                    if(this->KBoard->lenDataBuf==2)
                    {
                      this->LCD->sendMessage(0x80,0,"",0xC9,7,"(00-59)", false);
                      this->LCD->setAddr(0xC6);
                    }
                    if(this->KBoard->lenDataBuf>=4)
                    {
                      this->KBoard->lenDataBuf=4;
                      this->LCD->setAddr(0x93);
                      this->LCD->setCaracter('#');
                      this->LCD->setAddr(0xD1);
                      this->LCD->setCaracter('O');
                      this->LCD->setCaracter('K');
                      this->LCD->setCaracter('?');
                    }
                  }
                  else
                  {// Establece los valores seleccionados por teclado en el registro del reloj inmerso.
                    if(my >= this->Reloj->getNAlarms())
                    {
                      if(validateHourKB(this->KBoard->BufferKB))
                      {
                        setAlarm((char)my, this->KBoard->BufferKB);
                        this->saveToEEPROM();
                        my=0; this->exitMenu(); //my=0; mx=-1; IDMenu=0; this->LCD->exitLCDEdit(); menuSelection=false; this->printLCDDate(); // Sale de todo el menu
                      }
                      else
                      { IDMenu=255;
                      }
                    }
                    else
                    {
                      // Codigo para cambiar el horario de la alarma correspondiente
                      IDMenu=3121;
                      respIDMenu=IDMenu;
                      if(validateHourKB(this->KBoard->BufferKB))
                      {
                        setAlarm((char)my, this->KBoard->BufferKB);
                        my++;
                        this->LCD->sendMessage(0x83,11,"Horario #: ",0xC3,13,"HH:MM (00-23)", true); // menu para actualización de alarmas

                        if(my<=9)
                        {
                          this->LCD->setAddr(0x8F);
                          this->LCD->setCaracter(0x30+my);
                        }
                        else
                        {
                          sprintf(&this->KBoard->BufferKB[0],"%2d",my);
                          this->LCD->setAddr(0x8E);
                          this->LCD->setCaracter(this->KBoard->BufferKB[0]);
                          this->LCD->setAddr(0x8F);
                          this->LCD->setCaracter(this->KBoard->BufferKB[1]);
                        }

                        this->LCD->setAddr(0xC3);
                        this->clearBufferKB();//this->Reloj->setNAlarms(mx);
                        this->KBoard->lenDataBuf=0;
                      }
                      else
                      {IDMenu=255;
                      }
                    }
                  }
                  break;
          case '*': //IDMenu=0; mx=-1; this->setMenu('#');// Sale de todo el menu
                    // Regreso del cursor
                    this->KBoard->lenDataBuf--;
                      if(this->KBoard->lenDataBuf<0)
                      {
                         this->KBoard->lenDataBuf=0;
                      }
                      else
                      {
                        this->LCD->shiftLCDLeft();
                        if(this->KBoard->lenDataBuf==0)
                        {
                          this->LCD->setAddr(0xC3);
                        }
                        if(this->KBoard->lenDataBuf==3)
                        {
                        this->LCD->setAddr(0x93);
                        this->LCD->setCaracter(0x20);
                        this->LCD->setAddr(0xD1);
                        this->LCD->setCaracter(0x20);
                        this->LCD->setCaracter(0x20);
                        this->LCD->setCaracter(0x20);
                        this->LCD->setAddr(0xC7);
                        }
                        if(this->KBoard->lenDataBuf==1)
                        {
                          this->LCD->sendMessage(0x80,0,"",0xC9,7,"(00-23)", false); // menu para actualización de alarmas
                          this->LCD->setAddr(0xC4);
                        }
                      }
                    break;
          default:  // 1,2,3,4,5,6,7,8,9,0
                      this->KBoard->BufferKB[this->KBoard->lenDataBuf]=nKey;
                      this->LCD->setCaracter(nKey);
                      this->KBoard->lenDataBuf++;
                      if(this->KBoard->lenDataBuf==2)
                      {
                        this->LCD->sendMessage(0x80,0,"",0xC9,7,"(00-59)", false); // menu para actualización de alarmas
                        this->LCD->setAddr(0xC6);
                      }

                      if(this->KBoard->lenDataBuf>=4)
                      {
                        this->KBoard->lenDataBuf=4;
                        this->LCD->setAddr(0x93);
                        this->LCD->setCaracter('#');
                        this->LCD->setAddr(0xD1);
                        this->LCD->setCaracter('O');
                        this->LCD->setCaracter('K');
                        this->LCD->setCaracter('?');
                      }
                    break;
          }
          break;
////////////////////////////////////
   case 3221:

          switch(nKey)
          {
          case '*':
                   my++;
                   if(my>=3)
                   {
                     my=0;
                   }
                    mx=8;
                    if(my==0)
                    {
                      mx=6;
                    }
                    this->LCD->sendMessage(0x91,3,"   ",0x8B,mx,this->ModemGPRS->getAPNPorvByIndexList(my),false);
                    break;
          case '#':
                   this->ModemGPRS->setIDSP(my);
                   this->saveToEEPROM();
          case '4': this->exitMenu(); //mx=-1; IDMenu=0; this->LCD->exitLCDEdit(); menuSelection=false; this->printLCDDate();// Sale de todo el menu
                    break;
          }
          break;

////////////////////////////////////

   case 21:     // Selección de tiempo de muestreo de datalogger
          switch(nKey)
          {
          case '#':
                    this->exitMenu(); //mx=-1; IDMenu=0; this->LCD->exitLCDEdit(); menuSelection=false; this->printLCDDate();// Sale de todo el menu
                    break;
          }
          break;

////////////////////////////////////
   case 302:     // Selección de tiempo de muestreo de datalogger GSM
          switch(nKey)
          {
          case '*': // Select
                    my++;
                    if(my>=11)
                    { my=1;
                      switch(*this->MPG2->getNMedidores())
                      {
                      case 4: my=3; break;  // Para 60 min
                      case 3:
                      case 2: my=2;         // Para 30 y 60 min
                      default: break;
                      }
                    }
                    myLen=6;
                    mx=2;
//                    *(unsigned long int*)&this->KBoard->BufferKB[1]=(unsigned long int)*this->loggerSAT->getTimeLoggerSatByIndex(my);
                    switch(my)
                    {
                    case 0:
                                mx=0;
                                break;
                    case 1: case 2:
                                mx=1;
                                break;
                    default:
                                break;
                    }
                    sprintf((char*)&this->KBoard->BufferKB[0],"%2d %s",*this->loggerSAT->getTimeLoggerSatByIndex(my),this->loggerSAT->getTimeUnitsByIndex(mx));
                    if(my==3)
                    {this->KBoard->BufferKB[5]=0x20;}
                    this->LCD->sendMessage(0x87,myLen,&this->KBoard->BufferKB[0],0xC1,0,"", false);
                    this->LCD->configLCDEdit();
                    this->LCD->setAddr(0x88);
                    break;
          case '#':  // Enter
                    this->loggerSAT->setIndexSampleTime(my);
                    this->MPG2->restartWRPointers();
                    this->loggerSAT->clearNData();
                    this->saveToEEPROM();

          case '4': this->exitMenu(); //mx=-1; IDMenu=0; this->LCD->exitLCDEdit(); menuSelection=false; this->printLCDDate();// Sale de todo el menu
                    break;

          }
          break;
////////////////////////////////////
   case 303:     // Selección Habilitación / Deshabilitación de datalogger satelital
          switch(nKey)
          {
          case '*':
                    mx++;
                    if(mx>1)
                    {
                      mx=0;
                      this->LCD->sendMessage(0x80,20,"   Habilitar? #:Ent.",0xC1,0,"", false); // menu para actualización de alarmas
                    }
                    if (mx==1)
                    {
                      this->LCD->sendMessage(0x80,20,"Deshabilitar? #:Ent.",0xC1,0,"", false); // menu para actualización de alarmas
                    }
                    break;
          case '#': // Enter
                    if(mx)
                    this->loggerSAT->disableLogg();
                    else
                    this->loggerSAT->enableLogg();

                    this->MPG2->restartWRPointers();
                    this->loggerSAT->clearNData();

                    this->saveToEEPROM();
          case '4':
                    this->exitMenu(); //mx=-1; IDMenu=0; this->LCD->exitLCDEdit(); menuSelection=false; this->printLCDDate();// Sale de todo el menu
                    break;
          }
          break;
///////////////////////////////
   default: break;

   }
*/
}
/*********************************************************************/
/*void CModule::exitMenu()
{
  mx=-1; IDMenu=0; this->LCD->exitLCDEdit(); menuSelection=false;
  respIDMenu=IDMenu;
  this->printLCDDate();
}*/
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
/*void CModule::printLCDDate()
{
  char chrData[15];

  this->LCD->sendMessage(0x80,19,"Fecha:  /  /     D:",0xC0,18,"Rg:     :  :    H:", true);

  sprintf(&chrData[0],"%2d",this->Reloj->day);
  if(chrData[0]==0x20)
     chrData[0]=0x30;
  sprintf(&chrData[2],"%2d",this->Reloj->month);
  if(chrData[2]==0x20)
     chrData[2]=0x30;

  sprintf(&chrData[4],"%4d",this->Reloj->year);
  sprintf(&chrData[8],"%2d",this->Reloj->hour);
  if(chrData[8]==0x20)
     chrData[8]=0x30;

  sprintf(&chrData[10],"%2d",this->Reloj->min);
  if(chrData[10]==0x20)
     chrData[10]=0x30;
  sprintf(&chrData[12],"%2d",this->Reloj->seg);
  if(chrData[12]==0x20)
     chrData[12]=0x30;

  this->LCD->sendMessage(0x86,2,&chrData[0],0x89,2,&chrData[2], false);
  this->LCD->sendMessage(0x8C,4,&chrData[4],0xC6,2,&chrData[8], false);
  this->LCD->sendMessage(0xC9,2,&chrData[10],0xCC,2,&chrData[12], false);

  this->LCD->setAddr(0x93);
  this->LCD->setCaracter(0x30+*this->MPG2->getNMedidores());

  this->LCD->setAddr(0xD2);
  if(this->Reloj->getEnabledShedule())
  {
    sprintf(chrData,"%2d",this->Reloj->getNAlarms());
    this->LCD->setCaracter(chrData[0]);
    this->LCD->setCaracter(chrData[1]);
  }
  else
  {
    this->LCD->setCaracter('*');
  }

  this->LCD->setAddr(0xCF);
  if(this->ModemGPRS->isEnableSendingReport())
  {
    this->LCD->setCaracter(0x20);
  }
  else
  {
      this->LCD->setCaracter('*');
  }


}*/
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

  this->Mem->readAddr(logicAddrEEPROM,dataFlagEEPROMAddr,0x01,1,&EEPROMdata);  // Lee el registro de Reestablecimiento de información almacenada en EEPROM externa.

  if(EEPROMdata==dataMemID)
  {//Codigo para recuperar información de la memoria EEPROM correponientes a la cantidad de alarmas y sus valores corresondientes.
    startDataAddr=dataFlagEEPROMAddr+1;
    ptrBuffer=(char*)this->MPG2->UART->Buffer_Tx;
    this->Mem->readAddr(logicAddrEEPROM,startDataAddr,0x01,127,ptrBuffer); //

    ptrData=this->SysPass;          // Recupera password de usuario
    for(x=0;x<5;x++)
      ptrData[x]=*ptrBuffer++;

    ptrData=this->loggerSAT->getEnabledLoggAddr();
    for(x=0;x<4;x++)
      ptrData[x]=*ptrBuffer++;    // // Recupera variables del Logger para GPRS

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

    startDataAddr=dataFlagEEPROMAddr+128; // Lee siguiente bloque de memeoria EEPROM
    ptrBuffer=(char*)this->MPG2->UART->Buffer_Tx;
    this->Mem->readAddr(logicAddrEEPROM,startDataAddr,0x01,128,ptrBuffer); //

    ptrData=this->ModemGPRS->getAPN();          // Recupera password de usuario
    for(x=0;x<105;x++)
    {
      if(x==104)
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

  this->Mem->writeAddr(logicAddrEEPROM,dataFlagEEPROMAddr,0x01,&EEPROMdata,1);  // Almacena en memoria EEPROM el identificador de datos almacenados

  startDataAddr=dataFlagEEPROMAddr+1;
  this->Mem->writeAddr(logicAddrEEPROM,startDataAddr,0x01,(char*)&this->SysPass,5); // Almacena password de usuario del sistema
  startDataAddr+=5;

   this->Mem->writeAddr(logicAddrEEPROM,startDataAddr,0x01,this->loggerSAT->getEnabledLoggAddr(),4); // Almacena variables del Logger para GPRS
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

   startDataAddr=dataFlagEEPROMAddr+128; // inicia nuevo bloque en meemoria EEPROM
   this->Mem->writeAddr(logicAddrEEPROM,startDataAddr,0x01,this->ModemGPRS->getAPN(),105); // Almacena la configuración del módulo GSM (conexión a GPRS).
   startDataAddr+=105;





}
/*********************************************************************/
/*********************************************************************/
/*bool CModule::validateHourKB(char *Buffer)
{
  bool res=false;

  if (Buffer[0]>='0' && Buffer[0]<='1') // evalua horas
  {
    if (Buffer[1]>='0' && Buffer[1]<='9')
    {res=true;}
  }
  else
  {
    if(Buffer[0]=='2')
    {
      if (Buffer[1]>='0' && Buffer[1]<='3')
      {res=true;}
    }
  }

  if (Buffer[2]>='0' && Buffer[2]<='5' && res) // evalua minutos
  {
    if (Buffer[3]>='0' && Buffer[3]<='9')
    {res=true;}
    else
    {res=false;}
  }
  else
  {
    res=false;
  }

  if(!res)
  { this->LCD->exitLCDEdit();
    this->LCD->sendMessage(0x83,13,"ERROR en Hora",0xC5,10,"Presione #", true);} // menu para actualización de sensores
  return res;
}*/
/*********************************************************************/
/*bool CModule::validatePSIKB(char *Buffer)
{
  bool res=false;
  int x;

  for(x=0;x<4;x++)
  {
    if(Buffer[x]>='0' && Buffer[x]<='9')
    {res=true;}
    else
    {res=false; break;}
  }

  if(!res)
  {
    this->LCD->exitLCDEdit();
    this->LCD->sendMessage(0x81,17,"ERROR en Magnitud",0xC5,10,"Presione #", true); // menu para actualización de sensores
  }

  return res;
}*/
/*********************************************************************/
void CModule::clearBufferKB()
{
/*  int x;
  for(x=0;x<14;x++)
  this->KBoard->BufferKB[x]=0x30;
  */
}
/*********************************************************************/
/*void CModule::restoreFromEEPROM()
{
  char EEPROMdata=0x00;

  unsigned int startDataAddr=0x0001;
  int x;
  char *ptrData;

  this->Memory->readAddr(logicAddrEEPROM,dataFlagEEPROMAddr,1,&EEPROMdata);  // Lee el registro de Reestablecimiento de información almacenada en EEPROM externa.

  if(EEPROMdata==dataMemID)
  {//Codigo para recuperar información de la memoria EEPROM correponientes a la cantidad de alarmas y sus valores corresondientes.
    this->Memory->readAddr(logicAddrEEPROM,startDataAddr,1,&this->Reloj->nAlarms); // Lee la cantidad de alarmas y la almacena en el campo RAM del sistema correpondiente

    startDataAddr++;

    for(x=0;x<this->Reloj->nAlarms;x++)
    {
       ptrData=(char*)this->Reloj->alarms[x];
       this->Memory->readAddr(logicAddrEEPROM,startDataAddr,3,ptrData); // lee y almacena en la RAM correspondiente al sistema la información última de las alarmas.
       startDataAddr+=3;
    }

    startDataAddr=0x000B;
    ptrData=(char*)this->ADCRec->getMaxPressure(0);
    this->Memory->readAddr(logicAddrEEPROM,startDataAddr,5,ptrData);  // Lee la cantidad de dispositivos de medida de presión.
                                                                      // Offset de 4mA @ 2.5 Vmax y configuración de PSI's del canal 1 y 2

    this->ADCRec->configADC12();   // Reconfigura el modulo ADC12 con la cantidad de conalaes almacenados en memoria.
  }

}*/
/*********************************************************************/
/*void CModule::saveToEEPROM()
{
  //this->Memory->saveAlarmsEEPROM(this->Reloj);
  char EEPROMdata;
  unsigned int startDataAddr=0x0001;
  int x;//,y;
  char *ptrData;

  EEPROMdata=dataMemID;
  this->Memory->writeAddr(logicAddrEEPROM,dataFlagEEPROMAddr,&EEPROMdata,1);  // Almacena en memoria EEPROM el identificador de datos almacenados en la direción 0x0000
  this->Memory->writeAddr(logicAddrEEPROM,startDataAddr,&this->Reloj->nAlarms, 1); // Almacena la cantidad de alarmas

  startDataAddr++;

    for(x=0;x<this->Reloj->nAlarms;x++)
    {
       ptrData=(char*)this->Reloj->alarms[x];
       this->Memory->writeAddr(logicAddrEEPROM,startDataAddr,ptrData,3); // Almacena horas, minutos y segundos de cada alarma.
       startDataAddr+=3;
    }

    startDataAddr=0x000B;
    ptrData=(char*)this->ADCRec->getMaxPressure(0);
    this->Memory->writeAddr(logicAddrEEPROM,startDataAddr,ptrData,5); // Almacena cantidad de dispositivos de medida de presión.
                                                                      // Offset de 4mA @ 2.5 Vmax y configuración de PSI's del canal 1 y 2

}*/
/*********************************************************************/
/*bool CModule::displayShedule(char noReg)
{
  char Buffer[6];
  int x=3,y=0,z;

  char j=this->Reloj->getNAlarms();

  if(j && noReg<j)
  {
    if(noReg<9)
    {
      Buffer[y++]=0x31+noReg;
      z=0x8C;
    }
    else
    {
      sprintf(&Buffer[y],"%2d",noReg+1);
      x++;
      y+=2;
      z=0x8B;
    }
    Buffer[y++]='/';
    if(j<10)
    {
      Buffer[y++]=0x30+j;
    }
    else
    {
      sprintf(&Buffer[y],"%2d",j);
      y+=2;
      x++;
    }

    this->LCD->sendMessage(0x83,9,"Horario  ",z,x,&Buffer[0], true);

    sprintf(&Buffer[0], "%.2d", this->Reloj->alarms[noReg]->al_hour); // Horas
    sprintf(&Buffer[3], "%.2d", this->Reloj->alarms[noReg]->al_min); // Minutos
    Buffer[2]=':';

    this->LCD->sendMessage(0xC7,5,&Buffer[0],0xCB,0,"", false); // Imprime horas y minutos de la alarma
    return true;
  }
  else
  {return false;}

}*/
/*********************************************************************/
/*bool CModule::displayConfDevice(char noReg) // noReg 0...4...etc
{
  char Buffer[6];

  char j=*this->ItzyFlow->getNMedidores();

  if(j && noReg<j)
  {
    Buffer[0]=0x31+noReg;
    Buffer[1]='/';
    Buffer[2]=0x30+j;

    this->LCD->sendMessage(0x82,12,"Dispositivo ",0x8E,3,&Buffer[0], true);
    this->LCD->sendMessage(0xC0,5,"Addr:",0xCB,4,"Var:", false);

//////    sprintf(&Buffer[0], "%.4d", (*this->ItzyFlow->Flowmeter[noReg]->getStartAddressMB())+1); // Dirección de incio del registro Modbus
//    sprintf(&Buffer[3], "%.2d", this->Reloj->alarms[noReg]->al_min); // Minutos
//    Buffer[2]=':';

    this->LCD->sendMessage(0xC5,4,&Buffer[0],0xCF,5,(char*)MenuVarsReport_Devices[*this->ItzyFlow->Flowmeter[noReg]->getIDVariable()], false); // Imprime horas y minutos de la alarma
    return true;
  }
  else
  {return false;}

}*/
/*********************************************************************/
char* CModule::getSerialNumber()
{
  return &this->IDProduct[0];
}
/*********************************************************************/
/*bool CModule::evalPassword(char* Buffer, char MenuID)
{


}*/
/*bool CModule::displaySensorVal(char noReg)
{
  char Buffer[6];

  char j=*this->ADCRec->getNPressDevices();

  if(j && noReg<j)
  {
    Buffer[0]=0x31+noReg;
    Buffer[1]='/';
    Buffer[2]=0x30+j;

    this->LCD->sendMessage(0x83,11,"Pres. Max. ",0x8D,3,&Buffer[0], true);

    sprintf(&Buffer[0], "%.5d", *this->ADCRec->getMaxPressure(noReg)); // Horas

    this->LCD->sendMessage(0xC4,5,&Buffer[0],0xCB,3,"PSI", false); // Imprime horas y minutos de la alarma
    return true;
  }
  else
  {return false;}
}*/
/*********************************************************************/
/*void CModule::enterPasswordLCDPromt()
{
//  this->KBoard->lenDataBuf=0;
  this->LCD->sendMessage(0x82,16,"Ingrese Password",0xC8,4,"----", true);
  this->LCD->setAddr(0xC8);
  this->LCD->setCaracter('-');
  this->LCD->configLCDEdit();
  this->LCD->setAddr(0xC8);
}*/
/*********************************************************************/
/*bool CModule::validateAdressMB_KB(char *Buffer)
{
  bool res=true;
  int x;

  for(x=0;x<4;x++)
  {
    if (Buffer[x]<'0' && Buffer[x]>'9')  // evalua horas
    {
      res=false;
      break;
    }
  }

  if(!res)
  { this->LCD->exitLCDEdit();
    this->LCD->sendMessage(0x82,16,"ERROR de formato",0xC5,10,"Presione #", true);}
  return res;
}*/
/*********************************************************************/
/*void CModule::displayGSMVar(int nVar)
{
  int x1,y1,x2,y2;
  char *p_MSG1, *p_MSG2;

  switch(nVar)
  {
  case 0: // Desplegar Nombre Proveedor
         p_MSG2=this->ModemGPRS->getSPName();
         break;
  case 1: // Desplegar APN
         p_MSG2=this->ModemGPRS->getAPN();
         break;
  case 2: // Desplegar Usuario APN
         p_MSG2=this->ModemGPRS->getAPNUser();
         break;
  case 3: // Depslegar Password APN
         p_MSG2=this->ModemGPRS->getAPNPSW();
         break;
  case 4: // Desplegar DNS
         p_MSG2=this->ModemGPRS->getDNS();
         break;
  case 5: // Desplegar Puerto IP
         p_MSG2=this->ModemGPRS->getIPPort();
         break;
  }
  p_MSG1=(char*)MenuGSM_VerConn[nVar];
  y1=strlen(p_MSG1);
  x1= 0x80+((20-y1)/2);
  y2=strlen(p_MSG2);
  x2= 0xC0+((20-y2)/2);

    this->LCD->sendMessage(x1,y1,p_MSG1,x2,y2,p_MSG2, true);
//    this->LCD->sendMessage(0xC0,5,"Addr:",0xCB,4,"Var:", false);


}*/
/*********************************************************************/
