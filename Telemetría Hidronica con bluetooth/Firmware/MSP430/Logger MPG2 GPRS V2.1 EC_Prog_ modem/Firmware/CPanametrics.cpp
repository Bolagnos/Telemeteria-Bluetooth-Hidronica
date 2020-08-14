#include "CDevice.h"
//#include "string.h"
#include "string.h"
#include "math.h"

#define Num_ser_flash  0x1000  // Dirección del apuntador a número de serie del medidor en la región FLASH de Datos del microcontrolador.
/****************************************************************************************************/
const char VelocityU[3][9]={"Velocity","m/s","Ft/s",};

const char VolumetricU[30][11]={"Volumetric",
                           "A-F/D","A-F/H","A-F/M","A-F/S","A-I/D","A-I/H","A-I/M","A-I/S","Mf^3/d","ft^3/h",
                           "ft^3/m","ft^3/s","MGD","GAL/H","GAL/M","GAL/S","MBL/D","BBL/D","BBL/H","BBL/M",
                           "BBL/S","Mm^3/d","m^3/h","m^3/m","m^3/s","ML/D","L/H","L/M","L/S",
                          };
const char TotalizerU[13][10]={"FWD Total",
                          "ML","Mm^3","MBBL","BBL","MGAL","GAL","Mft^3","ft^3","AC-IN","AC-FT",
                          "m^3","L",
                          };
/*const char SignalStrg[2][16]={"UP Sig Strength","DN Sig Strength"
                          };*/

/****************************************************************************************************/
CPanametrics::CPanametrics()
{
  this->commCounter=0x00;
  this->requestData=false;
  this->LC=0x01;
  UART=new CUart1();
}
/****************************************************************************************************/
CPanametrics::~CPanametrics()
{
}
/****************************************************************************************************/
#define DLE 0x10
#define STX 0x02
#define SRC 0x01
#define ETX 0x03

const unsigned char PanametricsCommand[]={0x4F,//0x4F, // Comando para obtener las unidades de medida en el medidor.
                                          0x50, // Comando para obtener los valores de las variables del medidor.
                                          0x50,
//                                          0x18  // Read Error Flag List
                                         };
/****************************************************************************************************/
// Implementación de Funciones.
/****************************************************************************************************/
void CPanametrics::sendFrameToPanametrics(int Command)
{
//  if(this->requestData==false)
//  {
     //this->requestData=true;
     int mylen=0;
     this->commCounter=Command;
     mylen=this->assembleString(&Command);
     this->UART->startTx(this->UART,(char *)this->UART->Buffer_Tx,mylen); // Envía la cadena de datos por el puerto serie.
//  }
}
/****************************************************************************************************/
/****************************************************************************************************/
bool CPanametrics::evalMedidorRxString(char* BufferRx,int RxLength)
{
   bool response=false;
   char *RxCRC;
   unsigned int localCRC;
   int x;



   for(x=0;x<6;x++)     // Código para encontrar el primer byte cabecera del mensaje debido a bug del medidor AT868
   {
     if(BufferRx[x]==0x10)
     {
       BufferRx=&BufferRx[x];
       break;
     }
   }

   for(x=RxLength-1;x>RxLength-6;x--)     // Código para encontrar el ultimo byte cabecera del mensaje debido a bug del medidor AT868
   {
     if(BufferRx[x]==0x10)
     {
       RxLength=x-2;
       break;
     }
   }

   RxCRC=&BufferRx[RxLength];

/*   if(this->commCounter==1 && this->totalizerFwdDecPoint==0x03 && this->UART->strRxLen==26)
   {
      localCRC=this->CRC(BufferRx,RxLength-6); // solo si existe punto decimal a 3 digitos, si el comando es el #1 y si la cantidad de datos en RX es 26
   }
     */
   if(this->commCounter>=1 && RxLength>20)
   {
     localCRC=this->CRC(BufferRx,RxLength-(RxLength-20));
   }
   else
   {
    localCRC=this->CRC(BufferRx,RxLength);
   }

   if (*RxCRC==*((char*)&localCRC+1) && *(RxCRC+1)==*(char*)&localCRC)
   {
      response=true;
   }

   return response;
}
/****************************************************************************************************/
unsigned int CPanametrics::CRC(char* Buffer, int length)
{
    int x,y;

    unsigned int CRC=0;
    unsigned int tempReg=0;

    for(x=2;x<length;x++)
    {
       tempReg=Buffer[x];
       tempReg*=0x0100;
       for(y=0;y<8;y++)
       {

         if((CRC ^ tempReg)&0x8000)
         {
            CRC<<=1;
            CRC^=0x8005;
         }
         else
         {
            CRC<<=1;
         }
         tempReg<<=1;
       }

    }

    return CRC;
}
/****************************************************************************************************/
int CPanametrics::assembleString(int* Command)
{
   unsigned int calcCRC;
   int myLen;


     this->LC = 0x01 +(*Command & 0x00FF); // Linea a prueba (OK)

     this->UART->Buffer_Tx[0]=DLE;
     this->UART->Buffer_Tx[1]=STX;
     this->UART->Buffer_Tx[2]=this->LC;
     this->UART->Buffer_Tx[3]=SRC;
     this->UART->Buffer_Tx[4]=PanametricsCommand[*Command];

   switch (*Command)
   {
   case 0:
      this->UART->Buffer_Tx[5]=0x00;  // # canal 00
      this->UART->Buffer_Tx[6]=0x00;  // # offset 00
      this->UART->Buffer_Tx[7]=0x03;  // # entities to read 03
      myLen=8;
      break;
/*   case 2:
      this->UART->Buffer_Tx[5]=0x00;  // # canal 00
      this->UART->Buffer_Tx[6]=0x0B;  // # offset 0B
      this->UART->Buffer_Tx[7]=0x02;  // # entities to read 03
      myLen=8;
      break;
      */
   case 1:
      this->UART->Buffer_Tx[5]=0x03; // # 03 de variables a leer
      this->UART->Buffer_Tx[6]=0x00;  // # canal 00
      this->UART->Buffer_Tx[7]=0x00;  // # variable 00   // Velocity (Velocidad)
      this->UART->Buffer_Tx[8]=0x00;  // # canal 00
      this->UART->Buffer_Tx[9]=0x01;  // # variable 01   // Volumetric (Gasto)
      this->UART->Buffer_Tx[10]=0x00;  // # canal 00
      this->UART->Buffer_Tx[11]=0x02;  // # variable 02  // +Tot FWD (Volumen total de ida)
      myLen=12;
      break;
   case 2:
      this->UART->Buffer_Tx[5]=0x04; // # 04 de variables a leer
      this->UART->Buffer_Tx[6]=0x00;  // # canal 00
      this->UART->Buffer_Tx[7]=0x0B;  // # variable 11   // SS up (Signal Strength Up)
      this->UART->Buffer_Tx[8]=0x00;  // # canal 00
      this->UART->Buffer_Tx[9]=0x0C;  // # variable 12   // SS dw (Signal Strength Down)
      this->UART->Buffer_Tx[10]=0x00;  // # canal 00
      this->UART->Buffer_Tx[11]=0x17;  // # variable 22  // Q up
      this->UART->Buffer_Tx[12]=0x00;  // # canal 00
      this->UART->Buffer_Tx[13]=0x18;  // # variable 23  // Q down
      myLen=14;
      break;
   default: break;

   }
     calcCRC=this->CRC((char*)this->UART->Buffer_Tx, myLen);
     this->UART->Buffer_Tx[myLen++]=*((char*) &calcCRC+1);//MSB CRC
     this->UART->Buffer_Tx[myLen++]=*(char*) &calcCRC; //LSB CRC
     this->UART->Buffer_Tx[myLen++]=DLE;
     this->UART->Buffer_Tx[myLen++]=ETX;

     return myLen;
}
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
void CPanametrics::parsingData_Units(char* DataBuffer, int* NCommand)
{

   int x=0,y;
   char *zeroPointer,*unitPointer,*bufferPtr;
   char *startBuffer;
   double myTemp;

switch(*NCommand)
{
case 1: // Codigo para obtener los valores correspondientes a las variables Velocidad, Gasto y Volumen Fwd+ y Id de error.
        startBuffer=strchr(DataBuffer,PanametricsCommand[1]); // encuentra la direccion de la posición de comando respondido.
        startBuffer++;
        idError=(char)*startBuffer;
        startBuffer++;
        for(y=0;y<3;y++)    // # es la cantidad de datos a parsear
        {
          for(x=0;x<4;x++)
          {
            switch(y)
            {
            case 1:
                    flowRateReg[x]=startBuffer[3-x];
                    if(x==3)
                    {
                       myTemp=(*(unsigned long int*)&flowRateReg);
                       flowRateFT=myTemp*pow(10.0,-flowRateDecPoint);
                    }
                    break;
            case 2:
                    totalizerFwdReg[x]=startBuffer[3-x];
                    if(x==3)
                    {
                       myTemp=(*(unsigned long int*)&totalizerFwdReg);
                       totalizerFT=myTemp*pow(10.0,-totalizerFwdDecPoint);
                    }
                    break;
            case 0:
            default:
                    velocityReg[x]=startBuffer[3-x];
                    if(x==3)
                    {
                       myTemp=(*(unsigned long int*)&velocityReg);
                       velocityFT=myTemp*pow(10.0,-velocityDecPoint);
                    }
                    break;
            }
          }
          startBuffer+=5;
        }

        break;

case 2:// Codigo para obtener los valores correspondientes a las variables de Intensisdad de señales
        startBuffer=strchr(DataBuffer,PanametricsCommand[2]); // encuentra la direccion de la posición de comando respondido.
        startBuffer+=2;

        for(y=0;y<2;y++)    // # es la cantidad de datos a parsear
        {
          for(x=0;x<4;x++)
          {
            switch(y)
            {
            case 1:
                    SSDownReg[x]=startBuffer[3-x];
                    SSDown=(*(unsigned int*)&SSDownReg);
                    break;
            case 0:
            default:
                    SSUpReg[x]=startBuffer[3-x];
                    SSUp=(*(unsigned int*)&SSUpReg);
                    break;
            }
          }
          startBuffer+=5;
        }
   break;

case 0:
default:  // Codigo para identificación del tipo de unidades
   this->velocityUnits=0;
   this->flowRateUnits=0;
   this->totalizerFwdUnits=0;

   this->velocityDecPoint=0;
   this->flowRateDecPoint=0;
   this->totalizerFwdDecPoint=0;

   startBuffer=DataBuffer;
   zeroPointer=DataBuffer;

   do
   {
   bufferPtr=strstr(startBuffer,VelocityU[0]);
   if(bufferPtr)
   {
      for(x=1;x<sizeof(VelocityU);x++)
      {
         unitPointer=strstr(bufferPtr,VelocityU[x]);
        if(unitPointer)
        {velocityUnits=x; break;}
      }
      if(velocityUnits==x)
      {
        unitPointer+=strlen(VelocityU[x]);
        velocityDecPoint=*(char*)unitPointer;
        break;
      }
   }
   else
   {
      zeroPointer=strchr(startBuffer,0x00);
      startBuffer=zeroPointer+1;
   }
   }while(zeroPointer!=&DataBuffer[this->UART->strRxLen]);


   startBuffer=DataBuffer;
   zeroPointer=DataBuffer;

   do
   {
   bufferPtr=strstr(startBuffer,VolumetricU[0]);
   if(bufferPtr)
   {
      for(x=1;x<sizeof(VolumetricU);x++)
      {
        unitPointer=strstr(bufferPtr,VolumetricU[x]);
        if(unitPointer)
        {flowRateUnits=x; break;}
      }
      if(flowRateUnits==x)
      {
        unitPointer+=strlen(VolumetricU[x]);
        flowRateDecPoint=*(char*)unitPointer;
        break;
      }
   }
   else
   {
      zeroPointer=strchr(startBuffer,0x00);
      startBuffer=zeroPointer+1;
   }
   }while(zeroPointer!=&DataBuffer[this->UART->strRxLen]);


   startBuffer=DataBuffer;
   zeroPointer=DataBuffer;
   do
   {
   bufferPtr=strstr(startBuffer,TotalizerU[0]);
   if(bufferPtr)
   {
      for(x=1;x<sizeof(TotalizerU);x++)
      {
        unitPointer=strstr(bufferPtr,TotalizerU[x]);
        if(unitPointer)
        {totalizerFwdUnits=x; break;}
      }
      if(totalizerFwdUnits==x)
      {
        unitPointer+=strlen(TotalizerU[x]);
        totalizerFwdDecPoint=*(char*)unitPointer;
        break;
      }
   }
   else
   {
      zeroPointer=strchr(startBuffer,0x00);
      startBuffer=zeroPointer+1;
   }
   }while(zeroPointer!=&DataBuffer[this->UART->strRxLen]);

   break;
}

   for(x=0;x<this->UART->strRxLen;x++)
       DataBuffer[x]=0x00;


}
/****************************************************************************************************/
double* CPanametrics::getFlowrate()
{
  return &this->flowRateFT;
}
/****************************************************************************************************/
double* CPanametrics::getTotalizerFwd()
{
  return &this->totalizerFT;
}
/****************************************************************************************************/
char* CPanametrics::getUnitsFlowrate()
{
  return &this->flowRateUnits;
}
/****************************************************************************************************/
char* CPanametrics::getUnitsTotalizerFwd()
{
  return &this->totalizerFwdUnits;
}
/****************************************************************************************************/
double * CPanametrics::getFlowSpeed()
{
  return &this->velocityFT;
}
/****************************************************************************************************/
char * CPanametrics::getUnitsFlowSpeed()
{
   return &this->velocityUnits;
}
/****************************************************************************************************/
unsigned int * CPanametrics::getSSUp()
{
  return &this->SSUp;
}
/****************************************************************************************************/
unsigned int * CPanametrics::getSSDown()
{
  return &this->SSDown;
}
/****************************************************************************************************/
char * CPanametrics::getIDError()
{
  return &this->idError;
}
/****************************************************************************************************/
// resp=2 indica bandera para habilitar transmisiones via Satelital. (Finaliza secuencia de comandos requeridos).
//resp=1 indica finalización cualquier transmisión.
//resp=0 indica retransmiciones o indicación de tx del siguiente comando.

int CPanametrics::timeoutTxRx()
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

          if(this->commCounter<1)
          {
             this->commCounter++;
             this->sendFrameToPanametrics(this->commCounter);
          }
          else // Implementar fin de comandos
          {
            this->UART->UartError&=~0x01;
            //requestData==true
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

            resp=1;
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

