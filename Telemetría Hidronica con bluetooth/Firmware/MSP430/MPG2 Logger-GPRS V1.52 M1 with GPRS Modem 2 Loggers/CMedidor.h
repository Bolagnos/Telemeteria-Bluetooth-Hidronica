class CMedidor
{
private:

  //unsigned int flowRateFT;        // 2 bytes
  //unsigned long totalizerFT;      // 4 bytes
  //unsigned int levelFT;           // 2 bytes
  //unsigned char alarm;            // 1 byte
  //unsigned char nDolevas;         // 1 byte

  char idRegistro;                // Identificador de las variables a obtener por medidor (combinación de nivel, gasto, volumen)
                                  // 0x00 Not used
                                  // 0x01 gasto, volumen, batt, estado puerta (Reporte normal)
                                  // 0x02 gasto, nivel, batt, puerta Abierta Batt (Reporte de Alarma de apertura de puerta)

//////  unsigned int startAddrMODBUS;   // Dirección de inicio de las variables a leer por medidor
  unsigned long int p_memLoggerWrite;



public:
  CMedidor(char index);
  ~CMedidor();
public:
//  void setStartAddressMB(unsigned int startAddr);
//////  unsigned int* getStartAddressMB();
  void setIDVariable(char IDRegistro);
  char* getIDVariable();
  //void setRegFlowrate(unsigned int* FlowRate);
  //unsigned int* getRegFlowrate();
  //void setRegTotalizer(unsigned long* Totalizer);
  //unsigned long* getRegTotalizer();
  //void setRegLevel(unsigned int* Level);
  //unsigned int* getRegLevel();
  //void setRegAlarm(char* Alarm);
  //char* getRegAlarm();
  void setPointerWrite(unsigned long int pWrMem);
  unsigned long int* getPointerWrite();
  void addPointerWrite(int NBytes);
  const unsigned long int* getEEPROMstartAddr(int index);
  void restartWRpointer(int index);
  void restartWRpointerOffset(int index, int offset);
  //void setRegDolevas(char* NDolevas);
  //char* getRegDolevas();
};
