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
#define qtyDataEEPROM      0x200    // 512 bytes
#define dataFlagEEPROMAddr 0x10000  // Dirección de la bandera de detección de existencia de datos de configuración en EEPROM.
#define dataMemID          0x05A    // Dato de identificación de la existencia de datos validos en memeoria EEPROM.
#define logicAddrEEPROM    0x03     // Dirección lógica de la memoria EEPROM para datos de respaldo de sistema
#define flagChgBank       BIT7
#define flagBckSDMemWR    BIT2
#define flagWriteBckUp    BIT3
#define flagNewRegMP      BIT4
#define addrBackupSDMem   0x60600   // 512 bytes
class CEEPROM:public CDevice
{
public:
  CEEPROM();
  ~CEEPROM();
public:
   int secCounter;
   char flagsMem;   // BIT0: Memory bank 0 (bank size is 512 bytes)*
                    // BIT1: Memory bank 1 (bank size is 512 bytes)*
                    // BIT2: Flag to detect if there's a data backup to be send to SDMemory
                    // BIT3: Flag to detect if data writing is for Data back up to be send to SDMemory
                    // BIT4: Detects if there's a Flowmeter Register to store on SD Memory
                    // BIT7: Flag to detect change of data memory sector (bank 0 or bank 1)

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
  void setsBank(int bank);
  int getBank();
  void setChangeFlagBank();
  void clearChangeFlagBank();
  bool getChangeFlagBank();

  void setFlagBckUpSD();
  void clearFlagBckUpSD();
  bool getFlagBckUpSD();

  void enableFlagWritingBckUpSD();
  void disableFlagWritingBckUpSD();
  bool getFlagWritingBckUpSD();

  void setNewRegMPFlag();
  void clearNewRegMPFlag();
  bool getNewRegMPFlag();
};
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
   bool isHourFlagLoggerStoreForGPRS();
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
  //bool reqFromLogger;                         // Bandera de detección de requerimiento de datos para el logger (solicitud de datos del medidor)
  double timeRegCopy;
  int nData;
  bool alarmLoggSatActivated;
  bool flagReqStoreData;

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
  void readLoggedData(unsigned long int pAddr, char *Buffer, CEEPROM *memHandler);
 // bool isTimeSampleOn();
  int isTimeSampleOn(CRTC_3029_I2C* Reloj);
  //bool isReqLogger();
  //void setReqLoggerFlg();
  //void clearReqLoggerFlg();
  bool isMemFull();
  bool readLoggDataByBlk(char *Buffer, CEEPROM *memHandler, int NBlock);
  //char *getTimeLoggerByIndex(int index);
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

  bool isFlagStoreLogger();
  void clearFlagStoreLogger();
  void setFlagStoreLogger();

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
/*                  CMPG2 Derived Class from CDevice                        */
/*                  Comunicación con protocolo Modbus                       */
/****************************************************************************/

#define NumMedidores 0x02
#define posBuffData 40
const long int keyWord = 27816;

class CMPG2:public CDevice
{
public: // Local class objects
   CUart1 *UART;
public: // Local class variables
   int commCounter;     // Número de comando a enviar
   bool requestData;
   char nMedidores;
   char ID_Rep; // Tipo de reporte 0x31= '1' inicio de sistema '2'= reporte normal
   //bool flagsTXExt;  // 0: Bandera de detección si se envío un mensaje de respuesta a solicitud de configuración vía puerto serie para el medidor
                     //    (Es utilizado parano habilitar el WDT al terminar de enviar la trama)
                  // 1:
   bool flagEncrypted;
public:
   CMedidor *Flowmeter[NumMedidores];
   char serialNumber[12];  // Hasta 11 caracteres mas caracter nulo para el número de serie

public: // Constructors and destructors
   CMPG2();
   ~CMPG2();
public:  // Local class functions
  //void sendFrameToMPG2(int Command);
  void sendFrameToMPG2();
  //void sendFrameTx(char* BufferTx, int lenght);
  //int evalMedidorRxString(char* BufferRx,int RxLength,char* SerNo,CLogger* m_Logger,CRTC_3029_I2C *Reloj,CEEPROM *memory,char* dns,char* ipPort, char *apnName,char *apnUser, char *apnPsw, char *idSP);
  int evalMedidorRxString(char* BufferRx,int RxLength,char* SerNo,CLogger* m_Logger,CRTC_3029_I2C *Reloj,CEEPROM *memory);
//  void parsingData_Units(char* DataBuffer, char* BufferStore, int* NCommand, int RxLength);
  void parsingData_Units(char* DataBuffer, char* BufferStore, int* NCommand);
  //int timeoutTxRx(char* SerNo,CLogger* m_Logger,CRTC_3029_I2C *Reloj,CEEPROM *memory,char* dns,char *ipPort, char *apnName,char *apnUser, char *apnPsw, char *idSP);
  int timeoutTxRx(char* SerNo,CLogger* m_Logger,CRTC_3029_I2C *Reloj,CEEPROM *memory);
  char* getNMedidores();
  void setNMedidores(char nMed);
  void restartWRPointers(int offset);
  void reInitComms();
  void set_ID_Rep(char ID);
  char get_ID_Rep();
//  void setFlagTXExt();
//  void clearFlagTXExt();
//  bool getFlagTXExt();
  void clearCommCounter();
  int getCommCounter();
  void incCommCounter();
  bool isEncrypted();
  void setEncryptionMode();
  void setDesencryptedMode();
  char* getSerialNumberFM();
  void setFailRegisters(char* Buffer);

private:
char calcCRCMPG2(char* xBuffer, int length);
int assembleStringMPG2(int* Command);
char getCRCbuffer(char* xBuffer, int length);
char asctobcd(char asc);
//bool evalSN(char* bufferRef,char* buffertoEval);
void encryptData(long int *keyWord, char* eDataBuffer, int len, bool encrypt);

//void configEncrytionPort();
//bool isEncryptionMode();

};

/****************************************************************************/
/*               CSDMemoryCard Derived Class from CDevice                   */
/****************************************************************************/
#define flagStartTxCommand  BIT0
#define flagEndTxCommand    BIT1
#define flagStartRxCommand  BIT2
#define flagEndRxCommand    BIT3
#define blockSize 512

const char commandSPI[][6]={0x40, 0x00, 0x00, 0x00, 0x00, 0x95, // 1: CMD 0 Resets SD Card (GO_IDLE_STATE)
                            0x48, 0x00, 0x00, 0x01, 0xAA, 0x87, // 2: CMD 8 Interface codition (SEND_IF_COND)
                            0x7A, 0x00, 0x00, 0x00, 0x00, 0x95, // 3: CMD 58 Reads OCR register (READ_OCR)
                            0x7B, 0x00, 0x00, 0x00, 0x00, 0x85, // 4: CMD 59 Turns CRC on/off (CRC_ON_OFF)
                            0x77, 0x00, 0x00, 0x00, 0x00, 0x65, // 5: CMD 55 Next command is an App specific command (GEN_CMD)
                            0x69, 0x40, 0x00, 0x00, 0x00, 0x77, // 6: CMD 41 Capacity Support information and activates card's initialization process (SET_CLR_CARD_DETECT)
                            0x51, 0x00, 0x00, 0x00, 0x00, 0x95, // 7: CMD 17 Read single block (READ_SINGLE_BLOCK)
                            0x58, 0x00, 0x00, 0x00, 0x00, 0x95, // 8: CMD 24 Write single block (WRITE BLOCK)
                            0x4D, 0x00, 0x00, 0x00, 0x00, 0x95, // 9: CMD 13 Send status command (SEND_STATUS)
                            0x41, 0x40, 0x00, 0x00, 0x00, 0x95, // 10:CMD 1 Send status command (SEND_OP_COND)
                           };

const char dataBlank = 0xFF;

const char InitCommands[]={0,1,10,2,3,4,5,6};
const char ReadMemCommands[]={5,6,5,7,100};
const char WriteMemCommands[]={5,6,5,8,101,5,9}; //,102};


class CSDMemoryCard:public CDevice
{
public: // Local class objects
   CUart1 *UART;
public: // Local class variables

   long unsigned int RD_Addr;
   long unsigned int WR_Addr;
   char *commCounter;
   char *endCommCounter;
   char flagStateMsg;   // BIT0: Bandera de envío de solicitud de comando
                        // BIT1: Bandera de finalización de envío de comando
                        // BIT2: Bandera de inicio de envío de pulsos de CK para leer datos de la memoria SD
                        // BIT3: Bandera de finalización de lectura de datos de la memoria SD

   char flagWork;       // Bit 0: Initialization Process
                        // Bit 1: Read SD Memory Process
                        // Bit 2: Write SD Memory Process

   char *p_readData;     // Apuntador de lectura de un bloque de memeoria, apunta al primer dato si la llectura es válida, de otro modo devuelve cero.


public: // Constructors and destructors
   CSDMemoryCard(CUart1 *m_Uart);
   ~CSDMemoryCard();
public:  // Local class functions
  void writeConfig();
  void writeReadConfig();
  void writeMeterConfig();
  void sendFrameToMemorySPI();
  void sendFrameTx(char* BufferTx, int lenght);
  void clearCommCounter();
  void incCommCounter();
  char* getCommCounter();
  void setCommCounter(char* pos_command);
  bool isSPIEnabled();
  void setFlagStartTxCommand();
  void setFlagEndTxCommand();
  void setFlagStartRxCommand();
  void setFlagEndRxCommand();
  void clearFlagStartTxCommand();
  void clearFlagEndTxCommand();
  void clearFlagStartRxCommand();
  void clearFlagEndRxCommand();
  void clearFlagsRXTXCommand();
  bool isFlagStartTxCommand();
  bool isFlagEndTxCommand();
  bool isFlagStartRxCommand();
  bool isFlagEndRxCommand();
  char* startInitializeMem();
  char* startReadFromMem(long unsigned int l_addr);
  bool startWriteOnMem(long unsigned int l_addr);
  int getWorkProcess();
  void clearWorkProcess();
  char* getEndCommand();
  int* getCommTries();
  void incCommTries();
  void clearCommTries();
  void clearBufferSDTx();

private:
int assembleTxCommandSPI(char* Command);
int assembleTxFrametoRX(char* Command);
};
/****************************************************************************/
#define clustersPerMonth 20                  // 20*8 = 160 blocks of 512 bytes
//#define clustersPerYear  sectorsPerMonth*12  // 10*8*12 = 960 blocks of 512 bytes

class CFATDriver
{
public:
  CSDMemoryCard *SDMem;
private:
  char typeCode;
  unsigned long int LBA_begin;
  unsigned long int No_Sectors_Mem;

  unsigned int bytes_per_sector;
  char sector_per_cluster;
  unsigned int reserved_sectors;
  char fats_No;
  unsigned long int sectors_per_fat;
  unsigned long int root_directory_first_cluster;
  unsigned long int cluster_begin_lba;
public:
  unsigned long int l_FATsector;
  unsigned long int l_block_sector;   // Registro general para localizar o establecer un sector en memoria SD
  unsigned int l_pos_addr_sector;       // Registro general para localizar o establecer una dirección dentro de un sector en memoria SD

public:
  CFATDriver(CUart1* l_Uart);
  ~CFATDriver();
public:
  void readBootBlock();
  void readSystemPartition();
  void readFAT(unsigned long int n_Block);
  bool writeFAT(unsigned long int n_Block);
  char* findSpaceOnMemFolder(unsigned long int clusterNo);
  void createsNewYearFolder(char *p_data, char* year, CLogger *p_loggerMem, CEEPROM *p_I2CMem);
  unsigned long int getSizeOfFile(char* FileNameAddress);
  unsigned long int searchClusterOfFile(char* FileNameAddress);
  unsigned long int getLocalClusterOfFile(char* FileNameAddress);
  void setRegisterOnSDMem(char* FileName, unsigned long int l_NCluster, CMPG2* MPG2, CLogger* loggerSAT, CEEPROM* Mem);
  bool searchAndDeleteFolder(char* FolderNameYear, CLogger* loggerSAT, CEEPROM* Mem);
private:
  void clearBufferSDTx();
  unsigned long int searchFolder(char* FolderName, char iteration, char fileDataReq);

};

/****************************************************************************/
/*                      CGPRS Derived Class from CDevice                    */
/****************************************************************************/
// Señales para Bluetooth BL652
#define BL652_Reset       BIT5 // P2.5 RESET Bluetooth
#define BL652_nAutoRun    BIT6 // P2.6 nAutoRun Bluetooth

class CBluetooth:public CDevice
{
public:
  CBluetooth();
  ~CBluetooth();
public:
  void initPortSignals();
  void setRESET();
  void clearRESET();
  //bool isRESET();
  void resetProcedure();
};
/****************************************************************************/
/*                      CGPRS Derived Class from CDevice                    */
/****************************************************************************/
// Señales para módulo GPRS UC y EC20 de Quectel
#define W_ENABLE_GPRS     BIT3 // P1.3  // Control de paso a bajo consumo del módulo de GPRS (W-ENABLE)
#define ON_OFF_Modem_VCC  BIT6 // P1.6  // Control para encendido general del modem  GPRS (Alimentación, habilita relay de estado sólido)
#define ON_OFF_GPRS       BIT7 // P1.7  // Señal de encendido/apagado del módem GPRS (por su pin de control PERST#)
#define BitFlag_ONGPRS    BIT6 // P6.6  // Puerto de entrada para detección de que el módem se encuentra encendido


class CGPRS:public CDevice
{
 public:  // Local class Objects
  CUart0 *UART;
  CTimerB *timerReset;
  CBluetooth *BL652;
public:   // local class variables
  char APN_GPRS_SERV[30]; // Respaldar en EEPROM
  char APN_GPRS_USER[20]; // Respaldar en EEPROM
  char APN_GPRS_PSW[20];  // Respaldar en EEPROM
  char DNS[40];           // Respaldar en EEPROM
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
  bool flagsTXExt;  // 0: Bandera de detección si se envío un mensaje de respuesta a solicitud de configuración vía puerto serie para el medidor
                    //    (Es utilizado para no habilitar el WDT al terminar de enviar la trama)
                    // 1:
  char flags_req_info;  // BIT 0: Utilizado para habilitar el requerimiento via UART de lectura de una sección de la memoria SD (bloque de 512 Bytes)
                        // BIT 1: Utilizado para habilitar el requerimiento via UART para borrar un folder de archivos por año
                        // BIT 2: Utilizado para habilitar el requerimiento de lectura del medidor y envio vía GPRS hecho por una Unidad Verificadora
                        // BIT 3: Utilizado como bandera de solicitud de envío GPRS del reporte solicitado por la Unidad Verificadora
                        // BIT 4: Utilizado como bandera para identificar que se est+a enviano via GPRS un reporte solicitado por la Unidad Verificadora
                        // BIT 5: Disponible
                        // BIT 6: Disponible
                        // BIT 7: Utilizado como bandera para respaldar información introducida via Bluetooth
  bool validatePassword; // Bandera de validación del password para entrar al modo administrador.

public:   // Constuctors and destructors
   CGPRS();
   ~CGPRS();
public:   // Local class functions
   int evalWIND(char* l_Buffer, int len);
   void configGPRSCtrlPorts();
   void setAPNConf(char* ID);
   void sendFrameToGPRS(int Command);
   //void sendDataBasculaViaGPRS(CUart1* uartPort1);
   void sendStringToUART(char* BufferTx, int length);
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
//   unsigned long SC_OriginatedDefMsg(char* Buffer, CLogger* mLoggerSAT, CMPG2* mMPG2, CEEPROM *memHandler, char qtyMed);

   char calcCRC(char* xBuffer, int length);
   char getCRCbuffer(char* xBuffer, int length);
   char asctobcd(char asc);
   unsigned int calcFletcherCRC(char* Buffer, unsigned long length);
   unsigned long SetFormatMessage(char* Buffer, CLogger* mLoggerSAT, CMPG2* mMPG2, CEEPROM *memHandler, char* Nserie, int TZ);
   void OFFfromGPRSSession();
   void setIP_Port(char* IP_Port_Buff, int len);
   bool isIPSessionOpen();
   char* evalAndExecuteReqRemote(char* Buffer);

  ///////////////////////
   void configGPRSVars();
//   void setAPNConf(char* ID);
//   void setIP_Port(char* IP_Port_Buff, int len);
   char* getAPN();
   char* getAPNUser();
   char* getAPNPSW();
   char* getDNS();
   char* getIPPort();
   char* getSPName();
   char* getIDSP();
   void setIDSP(int ID);
   char* getAPNPorvByIndexList(int index);

   int evalBluetoothRxString(char* BufferRx,int RxLength,char* SerNo,CLogger* m_Logger,CLogger* m_LoggerGPRS,CRTC_3029_I2C *Reloj,CEEPROM *memory,CMPG2* mMPG2,char* Localization,char* l_RFC,char* mNoSerExt,char* mIDMedSM,char* mID_UV,char* mPassword,CFATDriver* mFatDrv);
   void setFlagTXExt();
   void clearFlagTXExt();
   bool getFlagTXExt();
   //int timeOUTExtRxBluetooth(CRTC_3029_I2C *myClock, char* SerialNumber, CLogger* SatLogg, CLogger* loggGPRS, CMPG2* mMPG2, CEEPROM *memHandler,char* Localization, char* mRFC, CFATDriver* mFatDrv);
   int timeOUTExtRxBluetooth(CRTC_3029_I2C *myClock, char* SerialNumber, CLogger* SatLogg, CLogger* loggGPRS, CMPG2* mMPG2, CEEPROM *memHandler, char* Localization, char* mRFC, char* mNoSerExt, char* mIDMedSM, char* mID_UV, char* mPassword, CFATDriver* mFatDrv);
   bool evalSN(char* bufferRef,char* buffertoEval);

   char* getFlags_reqInfo();

   void setValidatedPasswordFlag();
   void clearValidatedPasswordFlag();
   bool isValidatedPasswordFlag();
//   char calcCRC(char* xBuffer, int length);
//   char getCRCbuffer(char* xBuffer, int length);
//   char asctobcd(char asc);
//   unsigned int calcFletcherCRC(char* Buffer, unsigned long length);
};
/****************************************************************************/
class CButtonProg:public CDevice
{
private:
  CPortDetectBT *btnProg;
public:
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

