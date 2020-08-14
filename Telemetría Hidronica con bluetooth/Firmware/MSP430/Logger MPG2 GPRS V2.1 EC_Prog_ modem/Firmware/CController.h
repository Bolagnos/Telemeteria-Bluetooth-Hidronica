#include "msp430x16x.h"

class CController
{
public:
   CController();
   ~CController();
};
/*******************************************************************************************/
class CWdTimer:public CController
{
public:
  CWdTimer();
  ~CWdTimer();

  void configWdt(char EdgeSel, char NMISel,char WTSel, char Clksrc, int DivSel );

  static void stopWdt(void);
  static void startWdt(void);
  static void restartWdt(void);

  static void eIntWdt(void);
  static void dIntWdt(void);
  static bool isHold(void);

};
/*******************************************************************************************/
class CDMA:public CController
{
public:
  CDMA();
  ~CDMA();

  void configDMAfor_TXUARTs(void);
  void setDMASourceAddr_CH0(char* Buffer);
  void setDMASourceAddr_CH1(char* Buffer);
  void setTx0_Size(int size);
  void setTx1_Size(int size);
  void setDMA_CH0_EN();
  void setDMA_CH1_EN();
  void setDMA_CH0_REQ();
  void setDMA_CH1_REQ();
};
/*******************************************************************************************/
#define ON_OFF_RS232 BIT5  // P6.5 On/Off RS-232 Driver Signal
#define ENABLE_RS232 BIT4  // P6.4 /ENABLE RS-232 Reception Signal
#define SELECT_SW_PORT BIT2  // Selector for UART POR GPRS Modem  P3.2

class CUART:public CController
{
public:
        CDMA *m_DMA;
public:
 	unsigned int    portID;
	unsigned long   bps;
	float           clkfrec;
      	int strRxLen;
	int dataPtrRx;
        int dataPtrTx;
        int strTxLen;

	unsigned char   *Buffer_Tx_Start;
	char            sclk;
	char            statePort;
                  /*
                  Bit_0= Estado del puerto Abierto/Cerrado
                  Bit_1= Comunicaciones habilitadas Activada/Desactivada
                  Bit_2= Habilitación / deshabiliatación de reporte satelital. (solo para el producto 18 Telemetría Satelital para Scadapak)
                  */
        unsigned char   UartError;
                  /*
                  Bit_0= OverFlow_Error
                  */

        int extraTimeCounter;
        int commTries;
        int extraTimeout;
        int tries;
        CWdTimer* myWDT;
public:
   CUART();
   ~CUART();

      void clearRxBuffPointer(CUART *Uartx);
      void configUart(CUART* Uartx,unsigned int portID,unsigned long bps, char sclk);
      char openUart(CUART *Uartx);
      char isOpen(CUART *Uartx);
      void closeUart();
      unsigned int getPortID(CUART *Uartx);
      void setBaudRate(CUART *Uartx);
      void calcUARTModulationReg(double v_remain);
      void off_RS_232();
      void on_RS_232();
};

/*******************************************************************************************/
//#include "CTail.h"
#define buffer0_size 2100 // 2100
class CUart0:public CUART
{
public:
      CUart0();
      ~CUart0();
public:
      unsigned char Buffer_Tx[buffer0_size];
      unsigned char Buffer_Rx[buffer0_size];
public:
      void startTx(char *BufferTx,int strLen);
      int getBuffRxLen(CUart0 *Uartx);
      void clearBufferRx();
      CUart0* getUART();
      void setExtratimeOut(int x);
};
/*******************************************************************************************/
//#include "CTail.h"
#define buffer1_size 300
class CUart1:public CUART
{
public:
      CUart1();
      ~CUart1();
public:
      unsigned char   Buffer_Tx[buffer1_size];
      unsigned char   Buffer_Rx[buffer1_size];
public:
      void startTx(char *BufferTx,int strLen);
      int getBuffRxLen();
      CUart1 *getUART();

};
/*******************************************************************************************/
class CTimerA:public CController
{
public:
  CTimerA();
  ~CTimerA();
  int configTA(char srcClk1,char divisor,char modectrl);
  void clearTACounter(void);
  unsigned int getTACNT(void);
  void enableTAIE();
  void disableTAIE();
  void clearTAIFG();
};
/*******************************************************************************************/
/*******************************************************************************************/
class CTimerB:public CController
{
public:
  CTimerB();
  ~CTimerB();
  int configTB(char srcClk1,char divisor,char modectrl);
  void clearTBCounter(void);
  unsigned int getTBCNT(void);
  void enableTBIE();
  void disableTBIE();
  void clearTBIFG();
  void ConfigTB0asCapture();
  void EnableTBCCTL0_IE();
  void DisableTBCCTL0_IE();
};
/*******************************************************************************************/
class CParallelPort:public CController
{
public:
  CParallelPort();
  ~CParallelPort();
public:
  void configPortDataLCD(void);
  void configPortCtrlLCD(void);
  void sendMessage(char addr1,char length1,char* Message1,char addr2,char length2,char* Message2);
public:
  void commandInstr(char command);
  void writeInstr(char command);
  void setLCD_BL();
  void clearLCD_BL();

};
/*******************************************************************************************/
/*#define Row1  BIT3  // Renglon 1 B  // Row1 a Row4 se utilizarn como salidas en P1
#define Row2  BIT4  // Renglon 2 C
#define Row3  BIT5  // Renglon 3 L
#define Row4  BIT6  // Renglon 4 M

#define Col1  BIT0  // Columna 1 K  // Col1 a Col4 se utilizan como entradas en P1
#define Col2  BIT1  // Columna 2 D
#define Col3  BIT2  // Columna 3 E

class CPortKB:public CController
{
public:
  CPortKB();
  ~CPortKB();
public:

  void configPortRows(void);
  void configPortCols(void);
  void enableColInterrupt(char nPin);
  void disableColInterrupt(char nPin);
  void setColEdgeHtoL(char nPin);
  void setAllPortRows(void);

};*/
/*******************************************************************************************/
#define portSw BIT7  // Puerto entrada para detección de estado sel switch para la puerta en P6.7
class CPortDoorSw:public CController
{
public:
  CPortDoorSw();
  ~CPortDoorSw();
public:
  void configPort(void);
  bool readSwitchPort(void);
};
/*******************************************************************************************/
#define portBTNProg BIT0  // Puerto entrada para detección de estado sel switch para la puerta en P1.0
class CPortBtnProg:public CController
{
public:
  CPortBtnProg();
  ~CPortBtnProg();
public:
  void configPort(void);
  void enablePort();
  void disablePort();
  bool isBtnPressed();
};
/*******************************************************************************************/
/*#define onSystemLED   BIT4  // Puerto P4.5 indicador de encendido
#define BlueLED   BIT5  // Puerto P5.5 para encender Led Rojo
#define RedLED    BIT6  // Puerto P5.6 para encender Led Verde
#define GreenLED  BIT7  // Puerto P5.7 para encender Led Azul
*/
#define onSystemLED   BIT4  // Puerto P4.5 indicador de encendido
#define GreenLED  BIT5  // Puerto P5.5 para encender Led Rojo
#define RedLED    BIT6  // Puerto P5.6 para encender Led Verde
#define BlueLED   BIT7  // Puerto P5.7 para encender Led Azul

class CPortRGBLED:public CController
{
public:
  CPortRGBLED();
  ~CPortRGBLED();
public:
  void configPort(void);
  void offAllLED();
  void onRedLED();
  void onGreenLED();
  void onBlueLED();
  void onGreen_BlueLED();
  void onRed_BlueLED();
  void onRed_GreenLED();
  void onAllLED();
  void changeStateOnSystemLED();
};
/*******************************************************************************************/
#define nsamplesADC 4
class CADC12:public CController
{
public:
   int RPointer;          // Puntero para buffer de recolección
   char NPressDevices;    // Cantidad de sensores de presión a monitorear 0 a 2.
   char flagLogger;       // Registro banderas de control para Recoleción de datos
                          // 0x01=Recolección lista
                          // 0x02 ~ 0x080 Disponibles.
   unsigned int myBufferDataM0[nsamplesADC];  // Buffer de recoleccion Canal 0
//   unsigned int myBufferDataM1[nsamplesADC];  // Buffer de recoleccion Canal 1
public:
  CADC12();
  ~CADC12();
  void configADC12(int NChannels);
  void startADC12();
  void stopADC12();
  void getSample(struct ADC12* RecADC12);
  void clearADCflags();
  char isFlgLogger(struct ADC12* RecADC12);
  void setFlgLogger(struct ADC12* RecADC12);
  void clearFlgLogger(struct ADC12* RecADC12);
  long double calcPowerA1(struct ADC12* RecADC12, long double* FC, double* QLimit);
  long double calcPowerA2(struct ADC12* RecADC12);
  long double potProm(struct ADC12* RecADC12);
};
/****************************************************************************/
