#include "CDevice.h"

/****************************************************************************************************/

/****************************************************************************************************/
CSDMemoryCard::CSDMemoryCard(CUart1 *m_Uart)
{
  this->UART=m_Uart;
  this->flagStateMsg=0x00;
  this->flagWork=0x00;
  this->commCounter=0;
  this->endCommCounter=0;
  this->p_readData=0;
  this->RD_Addr=0x00000000;
  this->WR_Addr=0x00000000;
}
/****************************************************************************************************/
CSDMemoryCard::~CSDMemoryCard()
{
}
/****************************************************************************************************/
void CSDMemoryCard::writeConfig()
{
  this->UART->configDMAtoWriteMem();
}
/****************************************************************************************************/
void CSDMemoryCard::writeReadConfig()
{
  this->UART->configDMAtoReadMem();
}
/****************************************************************************************************/
void CSDMemoryCard::writeMeterConfig()
{
  this->UART->configDMAforFlowmeter();
}
/****************************************************************************************************/
// addr is exclusive to read or write a block memory.
void CSDMemoryCard::sendFrameToMemorySPI()
{
  int mylen=0;

  if(this->isFlagEndTxCommand())
  {// Establece la cantidad de pulsos de reloj para recibir datos de la memoria SD
    mylen=this->assembleTxFrametoRX(this->commCounter);
    this->setFlagStartRxCommand();
    if(*this->getCommCounter())
    {
      P5DIR|=BIT0;   // Down signal
    }
    this->UART->startTxRx((char*)&dataBlank,mylen,(char *)this->UART->SD_Buffer_Rx,mylen); // Envía la cadena de datos por el puerto serie.
  }
  else
  { // Establece comando a enviar a la memoria SD
    mylen=this->assembleTxCommandSPI(this->commCounter);
    this->setFlagStartTxCommand();
    if(*this->getCommCounter())
    {
      P5DIR|=BIT0;   // Down signal
    }

    //this->UART->startTx((char *)this->UART->Buffer_Tx,mylen); // Envía la cadena de datos por el puerto serie.
    this->UART->startTx((char *)this->UART->SD_Buffer_Tx,mylen,(char *)this->UART->SD_Buffer_Rx,mylen); // Envía la cadena de datos por el puerto serie.
  }
}
/****************************************************************************************************/
void CSDMemoryCard::sendFrameTx(char* BufferTx, int lenght)
{
     int mylen=0;
     mylen=this->assembleTxCommandSPI(this->commCounter);
     this->UART->startTx((char *)this->UART->SD_Buffer_Tx,mylen); // Envía la cadena de datos por el puerto serie.
}
/****************************************************************************************************/
int CSDMemoryCard::assembleTxCommandSPI(char* Command)
{
  int myLen=0;

  switch(this->getWorkProcess())
  {
  case BIT0:
            if(!((char)*Command))
            {
              for(;myLen<10;myLen++)
              {
                this->UART->SD_Buffer_Tx[myLen]=dataBlank;
              }
            }
            else
            {
              for(;myLen<6;myLen++)
              {
                this->UART->SD_Buffer_Tx[myLen]=commandSPI[*Command-1][myLen];
              }
            }
            break;
  case BIT1:
             if(((char)*Command)==7)
             {
               myLen=6;
             }
             else
             {
                for(;myLen<6;myLen++)
                {
                  this->UART->SD_Buffer_Tx[myLen]=commandSPI[*Command-1][myLen];
                }
             }
            break;
  case BIT2:
             if(((char)*Command)==8)
             {
               myLen=6;
             }
             else if(((char)*Command)==101)
             {
               myLen=blockSize+3; //1(start) + 512 (data) + 2 (CRC) + 1 (Confirmation)=516  ; 515 in this case
             }
             else
             {
                for(;myLen<6;myLen++)
                {
                  this->UART->SD_Buffer_Tx[myLen]=commandSPI[*Command-1][myLen];
                }
             }
            break;
  default: break;
  }

  /*
    if(*Command == 9)
    {
      this->UART->Buffer_Tx[myLen]=0xFE;
      myLen++;
      for(;myLen<516;myLen++)
      {
        this->UART->Buffer_Tx[myLen]=myLen;
      }
    }
    else
    {
      for(;myLen<6;myLen++)
      {
        this->UART->Buffer_Tx[myLen]=commandSPI[*Command-1][myLen];
      }
    }
  */
  return myLen;
}
/****************************************************************************************************/
int CSDMemoryCard::assembleTxFrametoRX(char* Command)
{
  int x;//,myLen;

  switch(*Command)
  {
  case 102:
  case 10:
  case 8:
  case 7:
  case 6:
  case 5:
  case 4:
  case 1:  x=3;   // R1 // No. de datos a recibir más dos
           break;
  case 3:
  case 2:  x=6; // No. de datos a recibir mas uno
           break;
  case 100:
           x=570; // 1(data Token)+512(Data)+2(CRC)=516 // No. de datos a recibir mas un, más 53 bytes mas para esperar el byte cabeceera (0xFE)
           break;
  case 9:
          x=4;
          break;
  case 101:
           x=4; // No de datos a recibir mas uno // 1 data token + 512 Data+ 2 CRC = 515 + 10
           break;
  default: x=0;
           break;
  }
/*
  for(myLen=0;myLen<x;myLen++)
  {
    this->UART->Buffer_Tx[myLen]=0x0FF;
  }
*/

  return x;
}
/****************************************************************************************************/
void CSDMemoryCard::clearCommCounter()
{
  this->commCounter=0;
}
/****************************************************************************************************/
void CSDMemoryCard::incCommCounter()
{
  this->commCounter++;
}
/****************************************************************************************************/
char* CSDMemoryCard::getCommCounter()
{
  return this->commCounter;
}
/****************************************************************************************************/
void CSDMemoryCard::setCommCounter(char* pos_command)
{
  this->commCounter=pos_command;
}
/****************************************************************************************************/
bool CSDMemoryCard::isSPIEnabled()
{
  if(this->UART->statePort & BIT3)
  {
    return true;
  }
  return false;
}
/****************************************************************************************************/
void CSDMemoryCard::setFlagStartTxCommand()
{
     this->flagStateMsg&=~(flagEndTxCommand | flagStartRxCommand | flagEndRxCommand);
     this->flagStateMsg|=flagStartTxCommand;
}
/****************************************************************************************************/
void CSDMemoryCard::setFlagEndTxCommand()
{
     this->flagStateMsg&=~(flagStartTxCommand | flagStartRxCommand | flagEndRxCommand);
     this->flagStateMsg|=flagEndTxCommand;
}
/****************************************************************************************************/
void CSDMemoryCard::setFlagStartRxCommand()
{
     this->flagStateMsg&=~(flagStartTxCommand | flagEndTxCommand | flagEndRxCommand);
     this->flagStateMsg|=flagStartRxCommand;
}
/****************************************************************************************************/
void CSDMemoryCard::setFlagEndRxCommand()
{
     this->flagStateMsg&=~(flagStartTxCommand | flagEndTxCommand | flagStartRxCommand);
     this->flagStateMsg|=flagEndRxCommand;
}
/****************************************************************************************************/
void CSDMemoryCard::clearFlagStartTxCommand()
{
     this->flagStateMsg&=~flagStartTxCommand;
}
/****************************************************************************************************/
void CSDMemoryCard::clearFlagEndTxCommand()
{
     this->flagStateMsg&=~flagEndTxCommand;
}
/****************************************************************************************************/
void CSDMemoryCard::clearFlagStartRxCommand()
{
     this->flagStateMsg&=~flagStartRxCommand;
}
/****************************************************************************************************/
void CSDMemoryCard::clearFlagEndRxCommand()
{
     this->flagStateMsg&=~flagEndRxCommand;
}
/****************************************************************************************************/
void CSDMemoryCard::clearFlagsRXTXCommand()
{
     this->flagStateMsg&=~(flagStartTxCommand | flagEndTxCommand | flagStartRxCommand | flagEndRxCommand);
}
/****************************************************************************************************/
bool CSDMemoryCard::isFlagStartTxCommand()
{
  if(this->flagStateMsg & flagStartTxCommand)
  {
    return true;
  }
  return false;
}
/****************************************************************************************************/

bool CSDMemoryCard::isFlagEndTxCommand()
{
  if(this->flagStateMsg & flagEndTxCommand)
  {
    return true;
  }
  return false;
}
/****************************************************************************************************/
bool CSDMemoryCard::isFlagStartRxCommand()
{
  if(this->flagStateMsg & flagStartRxCommand)
  {
    return true;
  }
  return false;
}
/****************************************************************************************************/

bool CSDMemoryCard::isFlagEndRxCommand()
{
  if(this->flagStateMsg & flagEndRxCommand)
  {
    return true;
  }
  return false;
}
/****************************************************************************************************/
char* CSDMemoryCard::startInitializeMem()
{
  this->UART->closeUart();
  this->flagWork=BIT0;
  this->clearCommTries();
  this->commCounter=(char*)&InitCommands[0];
  this->endCommCounter=this->commCounter + sizeof(InitCommands);
  this->writeConfig();
  if(!((char)*this->commCounter))
  {
    this->UART->openUart(this->UART,true);
  }
  this->clearFlagsRXTXCommand();
  this->p_readData=0;
  this->sendFrameToMemorySPI();
  while(this->getWorkProcess() & BIT0)
  {
  }

  return this->p_readData;
}
/****************************************************************************************************/
char* CSDMemoryCard::startReadFromMem(long unsigned int l_addr)
{
  unsigned long int timeout=0;
  this->RD_Addr=l_addr;
  this->flagWork=BIT1;
  this->clearCommTries();
  this->commCounter=(char*)&ReadMemCommands[0];
  this->endCommCounter=this->commCounter + sizeof(ReadMemCommands);
  this->writeConfig();
  this->clearFlagsRXTXCommand();
  this->p_readData=0;
  this->sendFrameToMemorySPI();
  /*while(this->getWorkProcess() & BIT1) // Aprox 16 seg
  {
  }*/
  while((this->getWorkProcess() & BIT1) && timeout<=20000)
  {
    timeout++;
  }
  if(timeout>=20000)
  {
    this->p_readData=0;
    this->clearCommTries();
    this->clearWorkProcess();
  }
  return this->p_readData;
}
/****************************************************************************************************/
bool CSDMemoryCard::startWriteOnMem(long unsigned int l_addr)
{
  unsigned long int timeout=0;
  this->WR_Addr=l_addr;
  this->flagWork=BIT2;
  this->clearCommTries();
  this->commCounter=(char*)&WriteMemCommands[0];
  this->endCommCounter=this->commCounter + sizeof(WriteMemCommands);
  this->writeConfig();
  this->clearFlagsRXTXCommand();
  this->sendFrameToMemorySPI();
  /*while(this->getWorkProcess() & BIT2)
  {
  }*/

  while((this->getWorkProcess() & BIT2) && timeout<=20000)
  {
    timeout++;
  }
  if(timeout>=20000)
  {
    this->clearCommTries();
    this->clearWorkProcess();
    return false;
  }
  else
  {
    return true;
  }
}
/****************************************************************************************************/
int CSDMemoryCard::getWorkProcess()
{
  return this->flagWork;
}
/****************************************************************************************************/
void CSDMemoryCard::clearWorkProcess()
{
  this->flagWork=0;
}
/****************************************************************************************************/
char* CSDMemoryCard::getEndCommand()
{
  return this->endCommCounter;
}
/****************************************************************************************************/
int* CSDMemoryCard::getCommTries()
{
  return &this->UART->commTries;
}
/****************************************************************************************************/
void CSDMemoryCard::incCommTries()
{
  this->UART->commTries++;
}
/****************************************************************************************************/
void CSDMemoryCard::clearCommTries()
{
  this->UART->commTries=0;
}
/****************************************************************************************************/
void CSDMemoryCard::clearBufferSDTx()
{
  int x;

  for(x=0;x<=512;x++)    // Copy Rx buffer to Tx buffer
  {
    this->UART->SD_Buffer_Tx[x]=0x00;
  }
}
/****************************************************************************************************/

/*
#include <iostream>
#include <cstdint>
#include <vector>
#include <array>

uint8_t CRCTable[256];

void GenerateCRCTable()
{
  int i, j;
  uint8_t CRCPoly = 0x89;  // the value of our CRC-7 polynomial

  // generate a table value for all 256 possible byte values
  for (i = 0; i < 256; ++i) {
    CRCTable[i] = (i & 0x80) ? i ^ CRCPoly : i;
    for (j = 1; j < 8; ++j) {
        CRCTable[i] <<= 1;
        if (CRCTable[i] & 0x80)
            CRCTable[i] ^= CRCPoly;
    }
  }
}

// adds a message byte to the current CRC-7 to get a the new CRC-7
uint8_t CRCAdd(uint8_t CRC, uint8_t message_byte)
{
    return CRCTable[(CRC << 1) ^ message_byte];
}

// returns the CRC-7 for a message of "length" bytes
uint8_t getCRC(uint8_t message[], int length)
{
  int i;
  uint8_t CRC = 0;

  for (i = 0; i < length; ++i)
    CRC = CRCAdd(CRC, message[i]);

  return CRC;
}

void PrintFrame(std::array<uint8_t, 6> & f)
{
  for (auto e : f)
    std::cout << std::hex << (int) e << " ";
  std::cout << std::endl;
}

int main()
{
  GenerateCRCTable();

  std::vector<std::array<uint8_t, 6>> CommandFrames;
  CommandFrames.push_back({{0x40, 0, 0, 0, 0, 0}});
  CommandFrames.push_back({{0x48, 0, 0, 1, 0xAA, 0}});
  CommandFrames.push_back({{0x69, 0x40, 0, 0, 0, 0}});
  CommandFrames.push_back({{0x77, 0x00, 0, 0, 0, 0}});
  CommandFrames.push_back({{0x7A, 0x00, 0, 0, 0, 0}});

  std::cout << "Command, Argument, CRC7" << std::endl;
  for (auto &Frame : CommandFrames) {
    Frame[5] = (getCRC(Frame.data(), 5) << 1) | 1;
    PrintFrame(Frame);
  }
}
*/
