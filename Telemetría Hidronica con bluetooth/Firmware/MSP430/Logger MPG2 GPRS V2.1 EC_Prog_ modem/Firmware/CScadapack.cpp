#include "CDevice.h"
//#include "string.h"
#include "string.h"
#include "math.h"

#define Num_ser_flash  0x1000  // Dirección del apuntador a número de serie del medidor en la región FLASH de Datos del microcontrolador.
/****************************************************************************************************/
/*const char VelocityU[3][9]={"Velocity","m/s","Ft/s",};

const char VolumetricU[30][11]={"Volumetric",
                           "A-F/D","A-F/H","A-F/M","A-F/S","A-I/D","A-I/H","A-I/M","A-I/S","Mf^3/d","ft^3/h",
                           "ft^3/m","ft^3/s","MGD","GAL/H","GAL/M","GAL/S","MBL/D","BBL/D","BBL/H","BBL/M",
                           "BBL/S","Mm^3/d","m^3/h","m^3/m","m^3/s","ML/D","L/H","L/M","L/S",
                          };
const char TotalizerU[13][10]={"FWD Total",
                          "ML","Mm^3","MBBL","BBL","MGAL","GAL","Mft^3","ft^3","AC-IN","AC-FT",
                          "m^3","L",
                          };
*/
/*const char SignalStrg[2][16]={"UP Sig Strength","DN Sig Strength"
                          };*/

/****************************************************************************************************/
CScadapack::CScadapack()
{
  char x;
  for(x=0;x<NumMedidores;x++)
  {
    this->Flowmeter[x]=new CMedidor(x);
  }
  this->commCounter=0x00;
  this->nMedidores=0x01; //  1= 1 medidor, 2= 2 medidores...etc.
  this->requestData=false;
  UART=new CUart1();
}
/****************************************************************************************************/
CScadapack::~CScadapack()
{
}
/****************************************************************************************************/
#define DLE 0x10
#define STX 0x02
#define SRC 0x01
#define ETX 0x03

const unsigned char PanametricsCommand[]={0x03, // Comando para obtener las variables para medidor 1
                                          0x03, // Comando para obtener las variables para medidor 2
                                          0x03, // Comando para obtener las variables para medidor 3
                                          0x03, // Comando para obtener las variables para medidor 4
                                          0x50,
                                         };
/****************************************************************************************************/
// Implementación de Funciones.
/****************************************************************************************************/
void CScadapack::sendFrameToPanametrics(int Command)
{
//  if(this->requestData==false)
//  {
     //this->requestData=true;
     int mylen=0;
     this->commCounter=Command;
     mylen=this->assembleString(&Command);
//     mylen=this->assembleString(&Command,0x01,0x03,0x0898,0x000A);
     this->UART->startTx(this->UART,(char *)this->UART->Buffer_Tx,mylen); // Envía la cadena de datos por el puerto serie.
//  }
}
/****************************************************************************************************/
/****************************************************************************************************/
bool CScadapack::evalMedidorRxString(char* BufferRx,int RxLength)
{
   bool response=false;
   char *RxCRC;
   unsigned int localCRC;
   int x;

   for(x=0;x<6;x++)     // Código para encontrar el primer byte cabecera del mensaje debido a bug del medidor AT868
   {
     if(BufferRx[x]==this->UART->Buffer_Tx[0]) // Evalua que la dirección física requerida sea la dirección que contesta
     {
       BufferRx=&BufferRx[x];
       break;
     }
   }
   if(x>=6)
   {
     return response;
   }

   if(BufferRx[1]!=this->UART->Buffer_Tx[1]) // Evalua que el comando solicitado haya sido el que se contestó.
   {
     return response;
   }

   RxLength-=2;
   RxCRC=&BufferRx[RxLength];
   localCRC=this->CRC(BufferRx,RxLength);

   if (*RxCRC==*((char*)&localCRC) && *(RxCRC+1)==*((char*)&localCRC+1))
   {
      response=true;
   }

   return response;
}
/****************************************************************************************************/
unsigned int CScadapack::CRC(char* Buffer, int length)
{
    int x,y;
    unsigned int CRC=0xFFFF;
    bool carry;

    for(y=0;y<length;y++)
    {
      CRC ^= (Buffer[y] & 0xFF);
      for (x=0; x<8; x++)
      {
        carry = CRC & 0x0001;
        CRC>>=1;
        if (carry)
        {
          CRC ^= 0xA001;
        }
      }
    }

    return CRC;
}
/****************************************************************************************************/
int CScadapack::assembleString(int* Command)
{
//   this->lenghtDataMODBUS

   unsigned int calcCRC;
   int myLen;
   int nData=5;   // Cantidad de datos de 16 bits a leer de los registros de ScadaPack

     this->UART->Buffer_Tx[0]=0x01;
     this->UART->Buffer_Tx[1]=0x03;
     this->UART->Buffer_Tx[2]=(*this->Flowmeter[*Command]->getStartAddressMB()&0xFF00)/0x0100;
     this->UART->Buffer_Tx[3]=*this->Flowmeter[*Command]->getStartAddressMB()&0x00FF;
     this->UART->Buffer_Tx[4]=(nData&0xFF00)/0x0100;
     this->UART->Buffer_Tx[5]=nData&0x00FF;

   switch (*Command) // Recordar que cada comando corresponde por cada petición de registros por cada medidor en ScadaPack (ya que las direcciónes de inicio de cada uno se encuentran corridas).
   {
   case 0:
   case 1:
   case 2:
   case 3:
            myLen=6;
            break;

   default: break;

   }
   calcCRC=this->CRC((char*)this->UART->Buffer_Tx, myLen);
   this->UART->Buffer_Tx[myLen++]=*(char*) &calcCRC;        //LSB CRC
   this->UART->Buffer_Tx[myLen++]=*((char*) &calcCRC+1);    //MSB CRC

     return myLen;
}
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
void CScadapack::parsingData_Units(char* DataBuffer, int* NCommand)
{
   int x=0,y;
   char *startBuffer;
   char myTemp;

   char m_t2;

switch(*NCommand)
{
case 0: // Codigo para obtener los valores correspondientes a las variables Velocidad, Gasto y Volumen Fwd+ y Id de error.
case 1:
case 2:
case 3:
        startBuffer=strchr(DataBuffer,PanametricsCommand[*NCommand]); // encuentra la direccion de la posición de comando respondido.
        startBuffer++;  //
        m_t2=(char)*startBuffer;
        myTemp=m_t2/2;    // Se divide la cantidad de bytes entre 2 debido a que se tienen registros enteros de 16 bits (Solo uno para volumen de 32 bits).
        startBuffer++;
        for(x=0;x<myTemp;x++)    // # es la cantidad de datos a parsear
        {
            switch(x)
            {
            case 1:                                           // Obtiene Registro de Volumen 32 bits
                    for(y=0;y<=1;y++)
                    totalizerFwdReg[y]=startBuffer[1-y];
                    startBuffer+=2;
                    for(y=0;y<=1;y++)
                    totalizerFwdReg[y+2]=startBuffer[1-y];
                    this->Flowmeter[*NCommand]->setRegTotalizer((unsigned long*) totalizerFwdReg);
                    x++;
                    break;
            case 3:                                           // Obtiene Registro de Nivel 16 bits
                    for(y=0;y<=1;y++)
                    levelReg[y]=startBuffer[1-y];
                    this->Flowmeter[*NCommand]->setRegLevel((unsigned int*)levelReg);
                    break;
            case 4:                                           // Obtiene Registro de Alarma 8 bits
                    for(y=0;y<=1;y++)
                    alarmReg[y]=startBuffer[1-y];
                    this->Flowmeter[*NCommand]->setRegAlarm((char*)alarmReg);
                    break;
            case 0:                                           // Obtiene Registro de Gasto 16 bits
                    for(y=0;y<=1;y++)
                    flowRateReg[y]=startBuffer[1-y];
                    this->Flowmeter[*NCommand]->setRegFlowrate((unsigned int*)flowRateReg);
                    break;
            default: break;
            }
          startBuffer+=2;
        }
        break;

default:  // Codigo para identificación del tipo de unidades
   break;
}

   for(x=0;x<this->UART->strRxLen;x++)
       DataBuffer[x]=0x00;


}
/****************************************************************************************************/
/****************************************************************************************************/
/****************************************************************************************************/
/*char * CScadapack::getUnitsFlowSpeed()
{
   return &this->velocityUnits;
}*/
/****************************************************************************************************/
/*unsigned int * CScadapack::getSSUp()
{
  return &this->SSUp;
}*/
/****************************************************************************************************/
/*unsigned int * CScadapack::getSSDown()
{
  return &this->SSDown;
}*/
/****************************************************************************************************/
/*char * CScadapack::getIDError()
{
  return &this->idError;
}*/
/****************************************************************************************************/
// resp=2 indica bandera para habilitar transmisiones via Satelital. (Finaliza secuencia de comandos requeridos).
//resp=1 indica finalización cualquier transmisión.
//resp=0 indica retransmiciones o indicación de tx del siguiente comando.

int CScadapack::timeoutTxRx()
{
  int resp=0;

  if(this->UART->statePort&0x02)  // Comunicaciones Tx o Rx del medidor Panametrics DF868?
  {
    this->UART->myWDT->stopWdt();
    this->UART->statePort&=~0x02;

    if (this->UART->UartError==0x01)  // Verifica si existe Error de Over flow del buffer de Rx.
    {
      this->UART->clearRxBuffPointer(this->UART); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
      this->UART->UartError&=~0x01;
    }
    else
    {
      if(this->UART->strRxLen)// Se recibió una cadena de datos.
      {
         this->UART->Buffer_Rx[this->UART->strRxLen]=0x00; // Almacena en el ultimo byte de la cadena de datos recibidos el caracter 0.
/***********************************************************/
/*         if(this->commCounter==0)
         {
          this->UART->Buffer_Rx[this->UART->strRxLen-3]-=0x42; // linea de debugg debe de quitarse en la aplicación final
         }
         if(this->commCounter==2)
         {
          this->UART->Buffer_Rx[this->UART->strRxLen-4]-=0x62; // linea de debugg debe de quitarse en la aplicación final
         }
*/
/***********************************************************/
        if (this->evalMedidorRxString((char*)&this->UART->Buffer_Rx[0],this->UART->strRxLen)) // Se recibió la respuesta de manera correcta del módulo ?
        {
          this->parsingData_Units((char*)&this->UART->Buffer_Rx[0], &this->commCounter);

          this->UART->extraTimeCounter=0;
          this->UART->commTries=0;
          this->UART->clearRxBuffPointer(this->UART); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
          this->UART->strRxLen=0;

          if(this->commCounter<this->nMedidores-1) // Determina la cantidad de comandos a enviar
          {
             this->commCounter++;
             this->sendFrameToPanametrics(this->commCounter);
          }
          else // Implementar fin de comandos
          {
            this->UART->UartError&=~0x01;

            // Implementar código para almacenar en memoria EEPROM la información recolectada.

            // Fin de implemenación de código para almacenar información recolectada en memoria EEPROM
            resp=2;
          }

        }
        else  //No se recibió la respuesta correcta o falta información.
        {
          if(this->UART->extraTimeCounter<this->UART->extraTimeout)// Espera de tiempo extra en espera para recibir datos en caso que no se haya recibido ningún byte.
          {
            this->UART->statePort|=0x02;
            this->UART->extraTimeCounter++;
            this->UART->myWDT->restartWdt();
          }
          else
          {
            this->UART->clearRxBuffPointer(this->UART); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
            this->UART->extraTimeCounter=0;
            this->UART->commTries++;
            if(this->UART->commTries>this->UART->tries) // Cantidad de intentos de retransmisión en caso de que no haya recibido ningún byte en respuesta.
            {
            // Finaliza retransmisiones a Medidor
              this->UART->commTries=0;
              this->UART->strRxLen=0;
              this->UART->UartError&=~0x01;

              resp=1;
//              mySystem->Reloj->myTimer->enableTBIE();
            }
            else // Retransmite último comando solicitado.
            {
               this->sendFrameToPanametrics(this->commCounter);
            }
          }
        }
      }
      else
      {
        if(this->UART->extraTimeCounter<this->UART->extraTimeout)// Espera de tiempo extra en espera para recibir datos en caso que no se haya recibido ningún byte.
        {
          this->UART->statePort|=0x02;
          this->UART->extraTimeCounter++;
          this->UART->myWDT->restartWdt();
        }
        else
        {
          this->UART->clearRxBuffPointer(this->UART); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
          this->UART->extraTimeCounter=0;
          this->UART->commTries++;
          if(this->UART->commTries>this->UART->tries) // Cantidad de intentos de retransmisión en caso de que no haya recibido ningún byte en respuesta.
          {
            //Finaliza retransimisiones a Medidor
            this->UART->commTries=0;
            this->UART->strRxLen=0;
            this->UART->UartError&=~0x01;

            resp=3;
//            mySystem->Reloj->myTimer->enableTBIE();
          }
          else // Retransmite último comando solicitado.
          {
            this->sendFrameToPanametrics(this->commCounter);
          }
        }
      }

    }

  }
  return resp;
}
/***********************************************************/
char* CScadapack::getNMedidores()
{
  return &this->nMedidores;
}
/***********************************************************/
void CScadapack::setNMedidores(char nMed)
{
  this->nMedidores=nMed;
}
/***********************************************************/
void CScadapack::restartWRPointers()
{
  int x;
  for(x=0;x<4;x++)
  this->Flowmeter[x]->restartWRpointer(x);
}
/***********************************************************/
