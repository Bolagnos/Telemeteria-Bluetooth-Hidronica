#include "CController.h"
#include "CAlarms.h"
#include "CMedidor.h"
/****************************************************************************/
/*                          CDevice Parent Class                            */
/****************************************************************************/
class CDevice
{
//Attributes
public:
//   CUart *UART;
public:
   CDevice();
   ~CDevice();
};
/******************************************************************************/
#define dataFlagEEPROMAddr 0x10000 // Dirección de la bandera de detección de datos en EEPROM.
#define dataMemID 0x0A5           // Dato de identificación de la existencia de datos validos en memeoria EEPROM.
#define logicAddrEEPROM 0x03      // Dirección lógica de la memoria EEPROM para datos de respaldo de sistema
class CEEPROM:public CDevice
{
public:
  CEEPROM();
  ~CEEPROM();
public:
   int secCounter;
private:
  void initializeEEPROM();
//  int writeAddr(char devaddr,unsigned int startAddr,char *info, unsigned int length);
  void startEEPROM(char banco);
  void stopEEPROM(char banco);
  void sendBit(char cbit,char banco);
  void sendData(char data,char banco);
  char getACK(char banco);
  char readBit(char banco);
  char readData(char banco);
  void setACK(char typeACK,char banco);
//  char* readAddr(char devaddr,unsigned int startAddr, unsigned int length, char* Buffer);
public:
  int writeAddr(char devaddr, unsigned long int startAddr, char flagMSB, char *info, unsigned int length);
  char* readAddr(char devaddr, unsigned long int startAddr, char flagMSB, unsigned int length, char* Buffer);
};
/****************************************************************************/
/*class CLcd:public CDevice
{
private:
  CParallelPort *PortHandler;
  char BLCounter;
public:
  CLcd();
  ~CLcd();
public:

  void sendMessage(char addr1,char length1,char* Message1,char addr2,char length2,char* Message2, bool LCDClear);
private:
  void configLCDPort(void);
public:
  void configLCD(void);
  void configLCDEdit(void);
  void exitLCDEdit(void);
  void setCaracter(char cData);
  void shiftLCDLeft();
  void shiftLCDRight();
  void setAddr(char cAddr);
  void onBackLight();
  void offBackLight();
  void incBLCounter();
  char getBLCounter();
  void clearBLCounter();

};*/
/****************************************************************************/
/*                      CReloj Derived Class from CDevice                    */
/****************************************************************************/
#define NAlarms 24   // Especifica la cantidad de alarmas a gestionar
const char numMonth[]= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
const char numDays[]= {31,28,31,30,31,30,31,31,30,31,30,31};
/*
class CReloj:public CDevice
{
public:  // Local class Objects
   CTimerB *myTimer;
   CAlarms *alarms[NAlarms];  // 0: Alarma time out falla de variables calculadas; 1: Alarma reporte diario.

public:   // local class variables
   int year;       // Registros del reloj inmerso.
   char seg;
   char min;
   char hour;
   char day;
   char month;
   char nAlarms;
   bool alarmActivated;
   int  timeZone;
   bool regEnableOp;  // Bandera de habilitación o des habilitación de la funcionalidad de evaluar las alarmas.

public:   // Constuctors and destructors
   CReloj();
   ~CReloj();
public:   // Local class functions
   void incClock(char nseg);
   void incClockWLCD(CLcd *myLCD,char nseg);
   void setClockWLCD(CLcd *myLCD, char lday, char lmonth, int lyear, char lhour, char lmin, char lseg, bool TZ);
   void setClock(char lday, char lmonth, int lyear, char lhour, char lmin, char lseg, bool TZ);
   int compareRegAlarm();
   void setAlarm(char nAl,char nHour,char nMin);
   void startReloj();
   void stopReloj();
   bool activatedAlarm();
   void setTimeZone(int Data);
   void setNAlarms(char noAlarms);
   char getNAlarms();
   bool getEnabledShedule();
};*/
/****************************************************************************/
#define HdwAddrRTC  0x0B
class CRTC_3029_I2C:public CDevice
{
public:
   CTimerA *myTimer;
   CAlarms *alarms[NAlarms];  // 0: Alarma time out falla de variables calculadas; 1: Alarma reporte diario.
public:

   char day;
   char month;
   int  year;       // Registros del reloj inmerso (7 bytes)
   char hour;
   char min;
   char seg;

   char nAlarms;    // Respladar en EEPROM
   int  timeZone;   // Respladar en EEPROM

   bool alarmActivated;
   bool regEnableOp;  // Bandera de habilitación o des habilitación de la funcionalidad de evaluar las alarmas.
   char m_Buffer[10]; // Buffer de uso general de RTC

public:
  CRTC_3029_I2C(CEEPROM *memHandler);
  ~CRTC_3029_I2C();

public:
   void setHour_Date(CEEPROM *memHandler);
   char* getHour_Date(CEEPROM *memHandler);
   void initLocalHour_Date(CEEPROM *memHandler);
   void configTimerRTC(CEEPROM *memHandler);


   //void setRTCOnIntTime(CEEPROM *memHandler, CLcd *mLCD, bool displayIt, bool isUTC);
   void setRTCOnIntTime(CEEPROM *memHandler, bool isUTC);
   void stopRTCCLK(CEEPROM *memHandler);
   void startRTCCLK(CEEPROM *memHandler);


public:
   void incClock(char nseg);
//   void incClockWLCD(CLcd *myLCD,char nseg);
//   void setClockWLCD(CLcd *myLCD, char lday, char lmonth, int lyear, char lhour, char lmin, char lseg, bool TZ, bool isUTC);
   void setClock(char lday, char lmonth, int lyear, char lhour, char lmin, char lseg, bool TZ, bool isUTC);
   int compareRegAlarm();
   void setAlarm(char nAl,char nHour,char nMin);
   void startReloj();
   void stopReloj();
   bool activatedAlarm();
   void setTimeZone(int Data);
   int* getTimeZone();
   void setNAlarms(char noAlarms);
   char getNAlarms();
   bool getEnabledShedule();
   void hexToBcdRTC(int *Buffer);
//   void updateLCDDate(CLcd *myLCD);

};
/****************************************************************************/
class CLogger:public CDevice
{
private:
  //unsigned long int p_memLoggerWrite;         // Registro apuntador de escritura la memoria del Logger (Memoria EEPROM I2C)
  bool enabledLogg;                           // Registro de habilitación / deshabilitación del Logger;
  bool fullLoggMem;                           // Bandera de detección de fin de memoria.
//  unsigned long int sampleTime;               // Registro de definición de tiempo de muestreo en segundos
//  unsigned long int countSampleTime;          // Contador de tiempo de muestreo
  char limitTotalMemAddr;                     // Indice limitador de memoria
  char IDSampleTime;                          // ID de tiempo de muestreo 0: 0 seg, 1: 15 min, 2: 30 min, 3: 1hr

  unsigned long int p_memLoggerRead;          // Registro apuntador de lectura la memoria del Logger (Memoria EEPROM I2C)
  bool reqFromLogger;                         // Bandera de detección de requerimiento de datos para el logger (solicitud de datos del medidor)
  double timeRegCopy;
  int nData;
  bool alarmLoggSatActivated;

public:
  CLogger();
  ~CLogger();
public:

  void setLimitAddrMem(char Addr);
//  void setPointerWrite(unsigned long int pWrMem);
//  unsigned long int *getPointerWrite();
  void enableLogg();
  void disableLogg();
  bool isEnabledLogg();
//  unsigned long int *getDefSampleTime();
//  unsigned long int *getCounterSmplTime();
  void copyDateTime(CRTC_3029_I2C *Reloj);
  double *getCopiedDateTime();

public:
//  void setDefSampleTime(unsigned long int smplTime);
  void setDefSampleTime(char *Buffer);
//  void incCounterSmplTime();
//  void clearCounterSmplTime();
  void loggingData(unsigned long int pAddr, char *Buffer, int nDataLogg, CEEPROM *memHandler);
 // bool isTimeSampleOn();
  int isTimeSampleOn(CRTC_3029_I2C* Reloj);
  bool isReqLogger();
  void setReqLoggerFlg();
  void clearReqLoggerFlg();
  bool isMemFull();
  bool readLoggDataByBlk(char *Buffer, CEEPROM *memHandler, int NBlock);
  char *getTimeLoggerByIndex(int index);
  char *getTimeLoggerSatByIndex(int index);
  char *getTimeUnitsByIndex(int index);
  void setIndexSampleTime(char index);
  char* getIndexSampleTime();
  char* getEnabledLoggAddr();
  void clearNData();
  void incNData();
  char getNDataBCD();
  int* getPNDataBCD();
  void setNDataBrkLnk();

};
/****************************************************************************/
/*#define Lev_Full        1.70  // F: Full
#define Lev_Low         1.60  // M: Medium
#define Lev_Very_Low    1.51  // L: Low
#define Lev_Disch_Comm  1.50  // D: Dischcarged
*/
#define Lev_Full        0.704   // F: Full
#define Lev_Low         0.688   // M: Medium
#define Lev_Very_Low    0.674   // L: Low
#define Lev_Disch_Comm  0.670   // D: Dischcarged

#define NAnalogDevices 1
class CInputVoltmeter:public CDevice
{
private:
  CADC12 *m_ADC;
private:
  unsigned long int addVoltage[NAnalogDevices];
  unsigned int counterReg;
  double Vpot;
  unsigned int t_adcVoltages[NAnalogDevices];
  bool flgRequest;
public:
  CInputVoltmeter();
  ~CInputVoltmeter();
public:
  void startCollector();
  void stopCollector();
  void getSample();
  char isCollectedFlag();
  double getVpot();
  double getVoltageBatt();
  void setFlagReq();
  void clearFlagReq();
  bool isFlagReq();
  void incCounterReg();
  void clearCounterReg();
  bool incAndCompCounter();
};
/****************************************************************************/
/*                 CBascula Derived Class from CDevice                      */
/****************************************************************************/
/*class CBascula:public CDevice
{
public: // Local class objects
   CUart1 *UART;
public: // Local class variables
   int commCounter;
   bool requestData;

public: // Constructors and destructors
   CBascula();
   ~CBascula();
public:  // Local class functions
  void sendFrameToBascula(CUart1 * uartPort, int Command, CBascula* mybascula);
  unsigned char evalBasculaResponse(unsigned char* BufferRx, CRTC_3029_I2C *myClock);

};*/
/****************************************************************************/
/*               CPanametrics Derived Class from CDevice                    */
/****************************************************************************/
/*
class CPanametrics:public CDevice
{
public: // Local class objects
   CUart1 *UART;
public: // Local class variables
   int commCounter;
   bool requestData;
   char LC; //Contador número de comando para el protocolo de comunicaciones de Panametrics.

private:
   char velocityReg[4];
   char velocityUnits;
   char velocityDecPoint;

   char flowRateReg[4];
   char flowRateUnits;
   char flowRateDecPoint;

   char totalizerFwdReg[4];
   char totalizerFwdUnits;
   char totalizerFwdDecPoint;

   double velocityFT;
   double flowRateFT;
   double totalizerFT;

   char SSUpReg[4];
   unsigned int SSUp;

   char SSDownReg[4];
   unsigned int SSDown;

   char idError;

public: // Constructors and destructors
   CPanametrics();
   ~CPanametrics();
public:  // Local class functions
  void sendFrameToPanametrics(int Command);
  bool evalMedidorRxString(char* BufferRx,int RxLength);
  void parsingData_Units(char* DataBuffer, int* NCommand);
  double *getFlowrate();
  double *getTotalizerFwd();
  char *getUnitsTotalizerFwd();
  char *getUnitsFlowrate();
  double *getFlowSpeed();
  char *getUnitsFlowSpeed();
  unsigned int *getSSUp();
  unsigned int *getSSDown();
  char *getIDError();


  int timeoutTxRx();

private:
unsigned int CRC(char* Buffer, int length);
int assembleString(int* Command);

};
*/
/****************************************************************************/
/*                CScadapack Derived Class from CDevice                     */
/*                  Comunicación con protocolo Modbus                       */
/****************************************************************************/
/*
#define NumMedidores 0x04

class CScadapack:public CDevice
{
public: // Local class objects
   CUart1 *UART;
public: // Local class variables
   int commCounter;
   bool requestData;
   char nMedidores;
private:
   char levelReg[4];
   char flowRateReg[4];
   char totalizerFwdReg[4];
   char alarmReg[2];
public:
   CMedidor *Flowmeter[NumMedidores];

public: // Constructors and destructors
   CScadapack();
   ~CScadapack();
public:  // Local class functions
  void sendFrameToPanametrics(int Command);
  bool evalMedidorRxString(char* BufferRx,int RxLength);
  void parsingData_Units(char* DataBuffer, int* NCommand);
  int timeoutTxRx();
  char* getNMedidores();
  void setNMedidores(char nMed);
  void restartWRPointers();

private:
unsigned int CRC(char* Buffer, int length);
int assembleString(int* Command);


};*/
/****************************************************************************/
/*                CScadapack Derived Class from CDevice                     */
/*                  Comunicación con protocolo Modbus                       */
/****************************************************************************/

#define NumMedidores 0x01
const long int keyWord = 27816;

class CMPG2:public CDevice
{
public: // Local class objects
   CUart1 *UART;
public: // Local class variables
   int commCounter;
   bool requestData;
   char nMedidores;
   char ID_Rep; // Tipo de reporte 0x31= '1' inicio de sistema '2'= reporte normal
   bool flagsTXExt;  // 0: Bandera de detección si se envío un mensaje de respuesta a solicitud de configuración vía puerto serie para el medidor
                     //    (Es utilizado parano habilitar el WDT al terminar de enviar la trama)
                  // 1:
   bool flagEncrypted;
/*private:
   char levelReg[4];
   char flowRateReg[4];
   char totalizerFwdReg[4];
   char alarmReg[2];*/
public:
   CMedidor *Flowmeter[NumMedidores];
   char serialNumber[9];

public: // Constructors and destructors
   CMPG2();
   ~CMPG2();
public:  // Local class functions
  void sendFrameToMPG2(int Command);
  void sendFrameTx(char* BufferTx, int lenght);
  int evalMedidorRxString(char* BufferRx,int RxLength,char* SerNo,CLogger* m_Logger,CRTC_3029_I2C *Reloj,CEEPROM *memory,char* dns,char* ipPort, char *apnName,char *apnUser, char *apnPsw, char *idSP);
  void parsingData_Units(char* DataBuffer, char* BufferStore, int* NCommand);
  int timeoutTxRx(char* SerNo,CLogger* m_Logger,CRTC_3029_I2C *Reloj,CEEPROM *memory,char* dns,char *ipPort, char *apnName,char *apnUser, char *apnPsw, char *idSP);
  char* getNMedidores();
  void setNMedidores(char nMed);
  void restartWRPointers();
  void reInitComms();
  void set_ID_Rep(char ID);
  char get_ID_Rep();
  void setFlagTXExt();
  void clearFlagTXExt();
  bool getFlagTXExt();
  void clearCommCounter();
  int getCommCounter();
  void incCommCounter();
  bool isEncripted();
  void setEncryptionMode();
  void setDesencryptedMode();

private:
//unsigned int CRC(char* Buffer, int length);
char calcCRCMPG2(char* xBuffer, int length);
int assembleStringMPG2(int* Command);
char getCRCbuffer(char* xBuffer, int length);
char asctobcd(char asc);
//int strFloattoInt(char* m_Buffer);
bool evalSN(char* bufferRef,char* buffertoEval);
void encryptData(long int *keyWord, char* eDataBuffer, int len, bool encrypt);
void configEncrytionPort();
bool isEncryptionMode();

};
/****************************************************************************/
/*                      CGPRS Derived Class from CDevice                    */
/****************************************************************************/
class CGPRS:public CDevice
{
public:  // Local class Objects
  CUart0 *UART;
  CTimerB *timerReset;
public:   // local class variables
  char APN_GPRS_SERV[30]; // Respaldar en EEPROM
  char APN_GPRS_USER[20]; // Respaldar en EEPROM
  char APN_GPRS_PSW[20];  // Respaldar en EEPROM
  char DNS[26];           // Respaldar en EEPROM
  char IP_PORT[7];        // Respaldar en EEPROM
  char ID_SERV_PROV;      // Respaldar en EEPROM

  char stateGPRS;
                  /* Las banderas so n para detectar la secuencia de que el módem está listo para utilizarce.
                  Bit_0= Se solicitó respuesta vía comando AT, 1 = Se solicitó, 0 = No se ha solicitado
                  Bit_1= La sesión GPRS está abierta, 1 = Abierta, 0 = Cerrada
                  Bit_2= Bandera de Habilitación de envíos de reportes, 1= Habilitado, 0=Deshabilitado (No realiza el reporte y no lo envía).
                  Bit_3= Bandera de detección de no recepción de datos provenientes del módem GSM (No se recibió respuesta alguna)
                  Bit_4= Bandera de envío de reporte pero con solicitud de apagar el módem GSM
                  Bit_5= Bandera para estado de comunicaciones línea de comandos AT = 0 o en línea de datos GPRS = 1
                  Bit_6= Bandera de identificación de comando AT no respondido al estar prendido el módem e intentar apagarlo en modo de línea de datos
                  Bit_7=
                  */
  char state_GSM_Modem;
                  /* Las banderas son para detectar la secuencia de que el módem está listo para utilizarce ante mensajes no esperados (+WIND)
                 - Bit_0= El rack está detectado 1= Cerrado, 0 = Abierto
                 - Bit_1= Existencia de SIM  1 = Se inserto, 0 = Se removió
                 - Bit_2= Habilitación de todos los comandos AT, 1 = Habilitados, 0 = Deshabilitado
                 - Bit_3= La red es solo para llamadas de emergencia, 1 = LLamadas de emergencia, 0 = No hay llamadas de emergencia
                 - Bit_4= Se perdió la conexión a la red, 1 = Perdió la red, 0 = Red detectada
                  Bit_5=
                  Bit_6=
                  Bit_7=
                  */
  int commCounter;        // Contador de comando solicitado
  int endCommandIndex;    // Número de comando a pararse
  int counterTimeout;

public:   // Constuctors and destructors
   CGPRS();
   ~CGPRS();
public:   // Local class functions
   int evalWIND(char* l_Buffer, int len);
   void configGPRSCtrlPorts();
   void setAPNConf(char* ID);
   void sendFrameToGPRS(int Command);
   void sendDataBasculaViaGPRS(CUart1* uartPort1);
   void sendStringGPRS(char* myStr);
   void setupGPRS();
   void connectGPRS();
   //unsigned int evalGPRSResponse(int* commandSent, unsigned char* BufferRx, int* RxLength, CRTC_3029_I2C* Reloj, CEEPROM *memHandler, CLcd *myLCD);
   unsigned int evalGPRSResponse(int* commandSent, unsigned char* BufferRx, int* RxLength, CRTC_3029_I2C* Reloj, CEEPROM *memHandler);

   //int timeoutTxRx(CRTC_3029_I2C *myClock, CLcd *myLCD, char* SerialNumber, CLogger* SatLogg, CMPG2* mMPG2, CEEPROM *memHandler);
   int timeoutTxRx(CRTC_3029_I2C *myClock, char* SerialNumber, CLogger* SatLogg, CMPG2* mMPG2, CEEPROM *memHandler);

   void disableSendingReport();
   void enableSendingReport();
   bool isEnableSendingReport();
   char* getAddrFlagsUARTstate();
   void OffModemGPRS();
   void OnModemGPRS();
   void turnON_VCC_GPRS();
   void turnOFF_VCC_GPRS();
   bool isModemGPRSOn();
   unsigned long SC_OriginatedDefMsg(char* Buffer, CLogger* mLoggerSAT, CMPG2* mMPG2, CEEPROM *memHandler, char qtyMed);

   char calcCRC(char* xBuffer, int length);
   char getCRCbuffer(char* xBuffer, int length);
   char asctobcd(char asc);
   unsigned int calcFletcherCRC(char* Buffer, unsigned long length);
   unsigned long SetFormatMessage(char* Buffer, CLogger* mLoggerSAT, CMPG2* mMPG2, CEEPROM *memHandler, char* Nserie, int TZ);
   void OFFfromGPRSSession();
   void setIP_Port(char* IP_Port_Buff, int len);
   bool isIPSessionOpen();
   char* evalAndExecuteReqRemote(char* Buffer);

   char* getAPN();
   char* getAPNUser();
   char* getAPNPSW();
   char* getDNS();
   char* getIPPort();
   char* getSPName();
   char* getIDSP();
   void setIDSP(int ID);
   char* getAPNPorvByIndexList(int index);
};
/****************************************************************************/
/****************************************************************************/
/*                     CSat201C Derived Class from CDevice                  */
/****************************************************************************/
/*class CSatQ1000:public CDevice
{
public:  // Local class Objects
  CUart0 *UART;
public:   // local class variables


  unsigned int packet_length;
  char retry; // De 0 a 255
  char mha;   // De 1 a 255

  char stateSat201C;

                  //Bit_0= Conexion con servidor  0= No conectado, 1= Conectado
                  //Bit_1=
                  //Bit_2=
                  //Bit_3=
                  //Bit_4=
                  //Bit_5=
                  //Bit_6=
                  //Bit_7=

  int commCounter;
//        int startCommandIndex;
  int endCommandIndex;

  int nReportQty;
  int nReport;
  char Buffer[6];

public:   // Constuctors and destructors
   CSatQ1000();
   ~CSatQ1000();
public:   // Local class functions


   void setupSat201C();
//   void getTimeDateSat201C();
//   void connectSat201C();
   void sendFrameToSat201C(unsigned long mylen);
//   void sendReportSat201C(CPanametrics *pflowmeter, int Command);
//   void sendDataBasculaViaGPRS(CUart0* uartPort0, CUart1 * uartPort1);
//   void sendStringSat201C(CUart0 * uartPort0, char * myStr);
   unsigned char evalSatQ1000Response(unsigned char* BufferRx, int* RxLength, CRTC_3029_I2C *myClock, CEEPROM* Mem, CLcd* m_LCD);

   void changeDateGPS(CRTC_3029_I2C *myClock, CLcd *myLCD);
   void setKReport(char kreport);
   int getKReport();
   int timeoutTxRx(CRTC_3029_I2C *myClock, CLcd *myLCD, char* SerialNumber, CLogger* SatLogg, CItzyFlow* mItzyFlow, CEEPROM *memHandler);

   void setAlarm(char NRegister, char *Buffer, CRTC_3029_I2C *Reloj);

   unsigned long setLevelACKBuff(char* Buffer, unsigned char parameter, char statusCode);
   unsigned long setGetParameterBuff(char* Buffer, unsigned char parameter);
   unsigned long setSetParameterBuff(char* Buffer, unsigned char parameter, char* value, char NBytesValue);
   unsigned long setCommsCommandBuff(char* Buffer, char actionReq, unsigned char* valueByte, char idGateway);

   unsigned int calcFletcherCRC(char* Buffer, unsigned long length);

   unsigned long SC_OriginatedDefMsg(char* Buffer, CLogger* mLoggerSAT, CItzyFlow* mItzyFlow, CEEPROM *memHandler, char qtyMed);
   unsigned long linkLevelAcknowledgment(char* Buffer, unsigned char statusCode);

// Comandos adiconales para lecura de datalogger vía RS-232
   int answertoCommand (char * bufferRx, char* bufferTx,char* flashSerialNumber, CLogger* memLogg,CEEPROM *memHandler, CRTC_3029_I2C* Reloj,CLcd *lcd);
   char evalSN(char * IDBufferRx, char*IDBuffer);
   char calcCRC(char* xBuffer, int length);
   char getCRCbuffer(char* xBuffer, int length);
   char asctobcd(char asc);
//   void calcDateModemQ1000(unsigned int nWeeks, unsigned long nSeconds, CRTC_3029_I2C* clkRTC);
   void calcDateModemQ1000(unsigned int nWeeks, long nSeconds, CRTC_3029_I2C* clkRTC, CEEPROM* Mem, CLcd* m_LCD);
   int calcDay_Hours(long nSeconds, CRTC_3029_I2C* clkRTC);
   void disableSendingReport();
   void enableSendingReport();
   bool isEnableSendingReport();
   char* getAddrFlagsUARTstate();
   bool evalAndExecuteReqRemote(char* Buffer);
   void OnModemSat();
   void OffModemSat();
   bool isModemSatOn();

};*/
/****************************************************************************/
//#define KBBufferSize 6
class CButtonProg:public CDevice
{
private:
  CPortBtnProg *btnProg;
public:
   //char KBBuffer[KBBufferSize];
  char btnFlag;
               // Bit 0: Start programmming via RS-232 telemetry flag
               // Bit 1-7:
  //int counter;  // Registro contador de segundos
public:
  CButtonProg();
  ~CButtonProg();
private:
  void configBtnProg();
public:
  void enableButton();
  void disableButton();
  bool isButtonPressed();
  void setProgFlag();
  void clearProgFlag();
  bool isProgFlagActivated();
  //void clearCounterSeconds();
  //void incCounterSeconds();
  //bool isCounterAtLimit();

};
/****************************************************************************/
//#define KBBufferSize 6
/*class CKeyBoard:public CDevice
{

private:
  CPortKB *KBPHandler;

public:
   //char KBBuffer[KBBufferSize];
  char KBFlags;
               // Bit 0:    Bandera de habilitación (1) / deshabilitación (0) del Teclado
               // Bit 1-7:  Disponibles.
  char BufferKB[18];
  int lenDataBuf;


public:
  CKeyBoard();
  ~CKeyBoard();

  //void sendMessage(char addr1,char length1,char* Message1,char addr2,char length2,char* Message2, bool LCDClear);
private:

  void configKB();
public:
  void enableKBoard();
  void disableKBoard();
  char getKeyPushed();

};*/
/****************************************************************************/
class CDoor:public CDevice
{
public:
  CPortDoorSw *portDoor;
  char counterTime;
public:
  CDoor();
  ~CDoor();
  bool doorSwitch;       // Estado del switch para puerta del gabinete de telemetria
  bool evalIsDoorOpen();
  void setDoorOpenStateFlg();
  void setDoorCloseStateFlg();
  bool getDoorStateFlg();
  void incCounterTime();
  void clearCounterTime();
  bool isCounterFull();
};
/****************************************************************************/
class CLedRGB:public CDevice
{
public:
  CPortRGBLED *ledRGBPort;
  char counterTime;
public:
  CLedRGB();
  ~CLedRGB();
  void offAll();
  void onOnlyRed();
  void onOnlyGreen();
  void onOnlyBlue();
  void onMagenta();
  void onYellow();
  void onCyan();
  void onWhite();
  bool isTimeToLighting();
  void changeStateOnLED();
};
/****************************************************************************/
/*
class CSat201C:public CDevice
{
public:  // Local class Objects
  CUart0 *UART;
public:   // local class variables

  char stateSat201C;

                  //Bit_0= Conexion con servidor  0= No conectado, 1= Conectado
                  //Bit_1=
                  //Bit_2=
                  //Bit_3=
                  //Bit_4=
                  //Bit_5=
                  //Bit_6=
                  //Bit_7=

  int commCounter;
//        int startCommandIndex;
  int endCommandIndex;

  int nReportQty;
  int nReport;

public:   // Constuctors and destructors
   CSat201C();
   ~CSat201C();
public:   // Local class functions
   void configSat201C();

   void turnOnSat201C(void);
   void turnOffSat201C(void);

   void setupSat201C();
   void getTimeDateSat201C();
   void connectSat201C();
   void sendFrameToSat201C(int Command);
   void sendReportSat201C(CPanametrics *pflowmeter, int Command);
//   void sendDataBasculaViaGPRS(CUart0* uartPort0, CUart1 * uartPort1);
//   void sendStringSat201C(CUart0 * uartPort0, char * myStr);
   unsigned char evalSat201CResponse(int *commandSent, unsigned char* BufferRx, int* RxLength, CRTC_3029_I2C *myClock);

   void changeDateGPS(CRTC_3029_I2C *myClock, CLcd *myLCD);
   void setKReport(char kreport);
   int getKReport();
   int timeoutTxRx(CPanametrics *myFlowmeter, CRTC_3029_I2C *myClock, CLcd *myLCD);

   void setAlarm(char NRegister, char *Buffer, CRTC_3029_I2C *Reloj);

};*/
/****************************************************************************/




