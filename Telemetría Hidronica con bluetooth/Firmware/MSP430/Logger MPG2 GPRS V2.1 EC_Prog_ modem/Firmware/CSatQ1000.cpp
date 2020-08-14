#include "CDevice.h"
//#include "string.h"
#include "string"
#include "stdlib.h"

#define ON_OFF_SAT BIT7

const char commandHeader=0x85;

const char linkLevelACKCommand=0x01;
const char commsCommCommand=0x03;
const char getParameterCommand=0x12;
const char setParameterCommand=0x13;



const char ErrorTypeLevACK[]={0x00,   // No error
                              0x01,   // Buffer unavailable, wait 30 sconds then send again
                              0x02,   // Packet rejected, invalid checksum
                              0x03,   // Packet rejected, invalid parameter
                              0x04,   // Packet rejected, size esceds queue capacity
                              0x05,   // Packet rejected, ill-formed
                              0x06,   // Packet rejected, unrecognized packet type
                              0x07    // Packet rejected, duplicate packet sequence number
                              };

const char actionReqCommComm[]={0,   // Request all SC-Terminated messages/commands queued in ORBCOMM Gateway
                                1,   // Request all SC-Terminated messages/commands queued in ORBCOMM Gateway of size no greater than 150 bytes
                                2,   // Request all GlobalGrams queued in satellite
                                3,   // Request O/R indicator addresses (section 3.2, note 4)
                                4,   // Request status of SC-Originated message/report/GlobalGram identified by MHA message reference number in value field
                                5,   // Request status of SC-Originated message/report identified by ORBCOMM Gateway message reference number in value field
                                6,   // Request list of subjects of messages queued in ORBCOMM Gateway
                                7,   // Request a single SC-Terminated message, identified by index in list of subjects, indicated in the value field. (Index value is
                                     // inferred from posicion of message subject relative to other message subjects, starting with value 1. List of subjects obtained
                                     // from previous transmission of SC communications command with type_code = 6).
                                8,   // Delete a single SC-Terminated message, identified by index in list of subjects, indicated in the value field (see type_code = 7
                                     // above)
                                9,   // Request registration with ORBCOMM Gateway
                                10,11,12,13,14,15,  // Generic functions (see section 3.2, note 15)
                                16,  // Request stsus packet
                                17,  // Clear acctive message being transfered (either SC-Originated or SC-Terminated) between SC and ORBCOMM Gateway.
                                18,  // Clear SC-Originated message identified by mha_ref_num in value field
                                19,  // Clear all messages in SC-Originated queue
                                20,  // Clear all messages in SC-Terminated queue
                                21,  // Perform self-test (internal check memory, modem, etc.)
                                22,  // Perform local loop-back test (dummy packet sent internally through SC-nothing transmitted to Satellite)
                                23,  // Perform loop-back test with ORBCOMM Gateway (SC sends Report with O/R indicator=0. ORBCOMM Gatewaty sends Sommans with
                                     // first 5 Report bytes)
                                24,  // Power down until DTR transitions from incactive to active
                                25,  // Power down for value second
                                26,  // Power down for at least value seconds, then awaken with the rise of the next satellite pass
                                27,  // Send position report to ORBCOMM Gateway
                                28,  // Send orbital elements as an SC-Terminated message (NASA 2-line format, Apendix *) to the DTE
                                29,  // Force SC to next known downlink in system
                                48,  // and above = SC Manufacturer defined
                               };


/****************************************************************************************************/
/*********************************************************************/
CSatQ1000::CSatQ1000()
{
  this->stateSat201C=0x00;
  this->commCounter=0x00;
  this->nReportQty=0x01;
  this->nReport=0x00;
  this->retry=0x00;
  this->mha=0x000; //0x01;
  UART=new CUart0();
  this->enableSendingReport();
  this->UART->selectModem();

  P1OUT&=~ON_OFF_SAT;    // Salida en 0
  P1DIR&=~ON_OFF_SAT;   // Establece el puerto como entrada


/*  P1DIR|=ON_OFF_SAT;
  P1OUT|=ON_OFF_SAT;
  */
//  this->UART->selectLocalUART();    // Establece por default el switch de selección de puerto hacia el puerto local RS-232
}
/*********************************************************************/
CSatQ1000::~CSatQ1000()
{
}
/*********************************************************************/
/****************************************************************************************************/
// Implementación de Funciones.
/****************************************************************************************************/
/****************************************************************************************************/
/****************************************************************************************************/
/****************************************************************************************************/
void CSatQ1000::sendFrameToSat201C(unsigned long mylen)
{
   this->UART->selectModem();
   this->UART->startTx((char *)&this->UART->Buffer_Tx[0],mylen); // Envía la cadena de datos por el puerto serie.
}
/****************************************************************************************************/
/*void CSatamatics201C::sendDataBasculaViaSat201C(CUart0 * uartPort0, CUart1 * uartPort1)
{
     int mylen=0;

     mylen=strlen((char *)&uartPort1->Buffer_Rx[0]);

     uartPort0->startTx(uartPort0,(char *)&uartPort1->Buffer_Rx[0],mylen); // Envía la cadena de datos por el puerto serie.

}*/
/****************************************************************************************************/
// Nota: Solo se pueden enviar hasta 21 caracteres en formato hexadeximal en ASCII
//       para el modulo satelital de Satamatics.

/****************************************************************************************************/
/*void CSatQ1000::setupSat201C()
{
  this->commCounter=0;
  this->endCommandIndex=4;
  sendFrameToSat201C(this->commCounter);
}*/
/****************************************************************************************************/
/*void CSatQ1000::getTimeDateSat201C()
{
  this->commCounter=8;
  this->endCommandIndex=8;
  sendFrameToSat201C(this->commCounter);
}*/
/****************************************************************************************************/

/****************************************************************************************************/
//  response:
//  0: No valida la respuesta (requiere time out)
//  1: Valida la respuesta
//  2: Requiere timeout y retransimisión de la cadena
//  3: Valida la respuesta de comado de Rx remoto (Del centro de datos) y envía el ID que identifica el cambio de la variable del sistema solicitada.
//  4: Valida la respuesta de comando de Rx remoto (del centro de datos) y envía el ID para respuesta de datos vía satelita.

unsigned char CSatQ1000::evalSatQ1000Response(unsigned char* BufferRx, int* RxLength, CRTC_3029_I2C *myClock, CEEPROM* Mem, CLcd* m_LCD)
{
  unsigned char response=0x00;

// Código para Evaluar CRC
//
     switch(BufferRx[1])  // Comando de recepción en microcontrolador
     {
     case 0x01:               // Link Level Acknowledgment
              if(BufferRx[4]==ErrorTypeLevACK[0])
              {
                if(this->UART->Buffer_Tx[1]==0x03 && this->UART->Buffer_Tx[5]==0x13)
                {
                  this->stateSat201C|=0x04; // Habilita bandera para enviar el nuevo mensaje después de haber borrado los restantes de la cola en el módem satelital.
                }
                this->linkLevelAcknowledgment((char*)this->UART->Buffer_Tx, 0x00);
                response=0x01;
              }
              break;
     case 0x0B:               // System Response
              if(BufferRx[5]==0x01 || BufferRx[5]==0x02)
              {
                if(BufferRx[7]==0x0F || BufferRx[5]==0x02)
                {
                  if(BufferRx[5]==0x01) // El envío de datos fué exitoso por parte del modem satelital
                  {
//                    this->stateSat201C|=0x10; // Habilita bandera para solicitar la cantidad de mensajes en cola en módem satelital y poder deshabilitar el puerto serie al módem
                    this->stateSat201C|=0x40; // Habilita bandera para solicitar el tiempo en semanas UTC
                  }
                  this->linkLevelAcknowledgment((char*)this->UART->Buffer_Tx, 0x00);
                  response=0x01;
                }
                else  // si existe error de confirmación
                {
                  if(BufferRx[5]==0x01)
                  {
                    this->Buffer[0]=BufferRx[10];
                    this->Buffer[1]=BufferRx[11];
                    this->Buffer[2]=0x00;
                    this->Buffer[3]=0x00;
                    this->Buffer[4]=BufferRx[6];
                    this->stateSat201C|=0x08;
                    this->linkLevelAcknowledgment((char*)this->UART->Buffer_Tx, 0x00);
                    response=0x01;
                    //this->stateSat201C|=0x08;
                    //this->setCommsCommandBuff((char*)this->UART->Buffer_Tx, 5, (unsigned char*)Buffer, BufferRx[6]);
                    //response=0x0B;
                  }
                }
              }
              break;
     case 0x14:               // Parameter Response
              if(BufferRx[5]==0x00 && BufferRx[6]==0x16) // Evaluacion del parámetro de No. de mensajes en cola del modem satelital.
              {
                this->UART->statePort&=~0x10;
                if(this->stateSat201C & 0x10)
                {
                  this->stateSat201C&=~0x10;
                  if(!BufferRx[8]) // Deshabilitar la selección del puerto serie direccionado al módem
                  {
                    this->stateSat201C|=0x20;
                  }
                }
                else
                {

                  if(BufferRx[8]>0x04)  // Verifica si la cantidad de mensajes en cola supera al límite establecido (4 mensajes en cola)
                  {
                    this->OffModemSat();
                    this->stateSat201C|=0x02; // Habilita bandera de detección de mensajes en cola de espera en modem satelital y borrarlos
                  }
                  else
                  {
                    this->stateSat201C|=0x04; // Habilita bandera para enviar el nuevo mensaje
                  }
                }
              }
              else if(BufferRx[5]==0x00 && BufferRx[6]==0x17) // Evaluacion del parámetro de cantidad de semanas en UTC
              {
               *(unsigned int*)&this->Buffer[0]=*(unsigned int*)&BufferRx[8];
                this->stateSat201C|=0x80; // Habilita bandera para solicitar tiempo en segundos UTC
              }
              else if(BufferRx[5]==0x00 && BufferRx[6]==0x18) // Evaluacion del parámetro de cantidad e segundos en UTC
              {
                this->calcDateModemQ1000(*(unsigned int*)&this->Buffer[0], *(long*)&BufferRx[8], myClock, Mem, m_LCD);
                this->stateSat201C|=0x10; // Habilita bandera para solicitar la cantidad de mensajes en cola
              }
              else
              {}
              this->linkLevelAcknowledgment((char*)this->UART->Buffer_Tx, 0x00);
              response=0x01;
              break;

     case 0x0C:               // SC-Terminated Message
                if(this->evalAndExecuteReqRemote((char*)BufferRx))
                {response=0x02;}
                else
                {response=0x01;}
                this->linkLevelAcknowledgment((char*)this->UART->Buffer_Tx, 0x00);
                break;
     case 0x04:               // System Announcement
     case 0x05:               // Status
     case 0x0D:               // SC-Terminated User Command
     case 0x0E:               // SC-Terminated GlobalGram
     case 0x10:               // Position Status
     case 0x15:               // Satellite State Vector
     case 0x16:               // Satellite Orbital Elements
     case 0x17:               // Satellite Plane Orbital Elements
     default:response=0x01;
             break;
     }

  return response;
}
/****************************************************************************************************/
void CSatQ1000::changeDateGPS(CRTC_3029_I2C *myClock, CLcd *myLCD)
{
  char Buffer[4];
  char *mypointer=(char*)&this->UART->Buffer_Rx[0];
  mypointer=strstr((char *)&this->UART->Buffer_Rx[0],"tt ");

  sscanf(mypointer+3, "%2d", (int*)&Buffer[0]);
  myClock->hour=Buffer[0];
  sscanf(mypointer+6, "%2d", (int*)&Buffer[0]);
  myClock->min=Buffer[0];
  sscanf(mypointer+9, "%2d", (int*)&Buffer[0]);
  myClock->seg=Buffer[0];
  sscanf(mypointer+12, "%2d", (int*)&Buffer[0]);
  myClock->day=Buffer[0];
  sscanf(mypointer+15, "%2d", (int*)&Buffer[0]);
  myClock->month=Buffer[0];
  sscanf(mypointer+18, "%4d", (int*)&Buffer[0]);
  myClock->year=*(int*)&Buffer[0];

  myClock->setClockWLCD(myLCD,myClock->day, myClock->month, myClock->year, myClock->hour, myClock->min, myClock->seg, true, false);
}
/****************************************************************************************************/
void CSatQ1000::setKReport(char kreport)
{
  kreport++;
  this->nReportQty=kreport;
}
/****************************************************************************************************/
int CSatQ1000::getKReport()
{
  return this->nReportQty;
}
/****************************************************************************************************/
int CSatQ1000::timeoutTxRx(CRTC_3029_I2C *myClock, CLcd *myLCD, char* SerialNumber, CLogger* SatLogg, CItzyFlow* mItzyFlow, CEEPROM *memHandler)
{
  int resp=0;
  int IDEval;
  unsigned int dataSize;
//  char Buffer[4];
  if(this->UART->statePort&0x02)  // Comunicaciones Tx o Rx del módulo Satelital o puerto local?
  {

    this->UART->myWDT->stopWdt();
    this->UART->statePort&=~0x02;

    if (this->UART->UartError==0x01)  // Verifica si existe Error de Over flow del buffer de Rx.
    {
      this->UART->clearRxBuffPointer(this->UART); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
      this->UART->UartError&=~0x01;
      this->UART->statePort&=~0x02;
    }
    else
    {
      if(this->UART->isSwUARTModem()) // Se tiene habilitado el puerto del modem satelital?
      {
        if(this->UART->strRxLen)// Se recibió una cadena de datos?
        {
          IDEval=this->evalSatQ1000Response(this->UART->Buffer_Rx, &this->UART->strRxLen, myClock, memHandler, myLCD);
          if (IDEval) // Se recibió la respuesta de manera correcta del módulo Satelital?
          {
            // Adicionar lineas de configuración de registros vía puerto serie del modulo satelital.
            // Fin lineas de configuración de registros via puerto serie del modulo satelital

            this->UART->clearRxBuffPointer(this->UART); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
            this->UART->strRxLen=0;
            this->UART->clearBufferRx();

            switch(IDEval) // Id de comando de respuesta hacia el modem satelital
            {
            case 0x02:this->UART->statePort|=0x08; // bandera para poder repaldar información de solicitud remota o configuración remota
                      mItzyFlow->restartWRPointers();
                      SatLogg->clearNData();
            case 0x01:
                      this->UART->commTries=this->UART->tries;
                      this->UART->commTries++;
                      this->UART->extraTimeCounter=this->UART->extraTimeout;
                      break;
            default:
                      this->UART->extraTimeCounter=0;
                      this->UART->commTries=0;
                      break;
            }
            this->sendFrameToSat201C(*((unsigned int*)&this->UART->Buffer_Tx[2]));

          }
          else  //No se recibió la respuesta correcta o falta información.
          {
            if(!IDEval)
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
                this->UART->clearBufferRx();
                this->UART->extraTimeCounter=0;
                this->UART->commTries++;
                if(this->UART->commTries>this->UART->tries) // Cantidad de intentos de retransmisión en caso de que no haya recibido ningún byte en respuesta.
                {
                // Finaliza retransmisiones a GPRS
                  this->UART->commTries=0;
                  this->UART->strRxLen=0;
                  this->UART->UartError&=~0x01;
                  resp=1; //3// 1
                }
                else // Retransmite último comando solicitado.
                {
                  this->sendFrameToSat201C(*((unsigned int*)&this->UART->Buffer_Tx[2]));
                }
              }
            }
            else
            { // codigo para evaluar respuestas relacionadas a comandos externos (del servidor de datos).
              resp=IDEval;
              this->UART->clearRxBuffPointer(this->UART); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
              this->UART->clearBufferRx();
              this->UART->strRxLen=0;
            }
          }
        } // Fin if si se recibió una cadena de datos
        else // si no recibió una cadena de datos del modem satelital y se esperaba dependiendo el comando solicitado
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
              //this->UART->clearRxBuffPointer(this->UART); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
              this->UART->strRxLen=0;
              this->UART->clearBufferRx();
              //this->UART->extraTimeCounter=0;
              this->UART->commTries=0;

              if(this->stateSat201C & 0x02) // Evalua bandera de detección de mensajes en espera en modem satelital
              {
                this->OnModemSat();
                this->stateSat201C&=~0x02;
                this->Buffer[0]=0x00;
                this->Buffer[1]=0x00;
                this->Buffer[2]=0x00;
                this->Buffer[3]=0x00;
                this->setCommsCommandBuff((char*)this->UART->Buffer_Tx, 19, (unsigned char*)Buffer, 0x01); // Comando para borrar mensajes en cola
                this->sendFrameToSat201C(*((unsigned int*)&this->UART->Buffer_Tx[2]));
              }
              else if(this->stateSat201C & 0x04)
              {
                this->stateSat201C&=~0x04;
                dataSize=this->SC_OriginatedDefMsg((char*)this->UART->Buffer_Tx,SatLogg,mItzyFlow,memHandler,0);  // Comando para enviar el reporte actual
                if(dataSize)
                {
                  //this->stateSat201C|=0x40;
                  this->sendFrameToSat201C(dataSize);
                }
                else
                {     // No envía el reporte en caso de que no existan datos recolectados
                  this->stateSat201C|=0x10;
                  this->setGetParameterBuff((char*)this->UART->Buffer_Tx, 0x16); // Envía obtener la cantidad de mensajes en cola del modem satelital.
                  this->sendFrameToSat201C(*((unsigned int*)&this->UART->Buffer_Tx[2]));

                  //this->UART->UartError&=~0x01;
                  //resp=1;
                }
              }
              else if(this->stateSat201C & 0x08)
              {
                  this->stateSat201C&=~0x08;
                  this->setCommsCommandBuff((char*)this->UART->Buffer_Tx, 5, (unsigned char*)this->Buffer, this->Buffer[4]);
                  this->sendFrameToSat201C(*((unsigned int*)&this->UART->Buffer_Tx[2]));
              }
              else if(this->stateSat201C & 0x10)
              {
                  //this->stateSat201C&=~0x10;
                  this->setGetParameterBuff((char*)this->UART->Buffer_Tx, 0x16); // Envía obtener la cantidad de mensajes en cola del modem satelital.
                  this->sendFrameToSat201C(*((unsigned int*)&this->UART->Buffer_Tx[2]));
              }
              else if(this->stateSat201C & 0x20)    // Sale de la rutina de espera de tramas Rx o Tx del modem satelital
              {
                this->stateSat201C&=~0x20;
//                this->UART->selectLocalUART();
                this->UART->UartError&=~0x01;
                resp=1; // 1
              }
              else if(this->stateSat201C & 0x40)  // Pide tiempo en semanas UTC
              {
                  this->stateSat201C&=~0x40;
                  this->setGetParameterBuff((char*)this->UART->Buffer_Tx, 0x17); // Envía obtener la cantidad de mensajes en cola del modem satelital.
                  this->sendFrameToSat201C(*((unsigned int*)&this->UART->Buffer_Tx[2]));
              }
              else if(this->stateSat201C & 0x80)  // Pide tiempo en segundos
              {
                  this->stateSat201C&=~0x80;
                  this->setGetParameterBuff((char*)this->UART->Buffer_Tx, 0x18); // Envía obtener la cantidad de mensajes en cola del modem satelital.
                  this->sendFrameToSat201C(*((unsigned int*)&this->UART->Buffer_Tx[2]));
              }
              else
              { //Finaliza retransimisiones a GPRS en caso de que no exista respuesta de Tx del modem
                //this->UART->commTries=0;
                //this->UART->strRxLen=0;
                  if(this->UART->statePort & 0x10)
                  {
                    this->OffModemSat();
                    this->UART->statePort&=~0x10;
                  }
                  this->stateSat201C&=~0x20;
                  this->UART->UartError&=~0x01;

                  resp=1; // 1
              }
            }
            else // Retransmite último comando solicitado.
            {
              this->sendFrameToSat201C(*((unsigned int*)&this->UART->Buffer_Tx[2]));
            }
          }
        }
      }
      // Codigo para evaluar comandos vía puerto local RS-232
/*      else
      {

        if (this->calcCRC((char*)this->UART->Buffer_Rx,this->UART->strRxLen-4)==this->getCRCbuffer((char*)this->UART->Buffer_Rx, this->UART->strRxLen))
        {


            int mylen=answertoCommand ((char*)this->UART->Buffer_Rx, (char*)this->UART->Buffer_Tx,SerialNumber, memLogg, memHandler,myClock,myLCD);

            int mylen=0;  // R
            this->UART->extraTimeCounter=0;
            this->UART->commTries=0;
            this->UART->clearRxBuffPointer(this->UART); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
            this->UART->strRxLen=0;
            this->UART->clearBufferRx();

            if (mylen)
            {
               this->UART->startTx((char *)&this->UART->Buffer_Tx[0],mylen); // Envía la cadena de datos por el puerto serie.
            }
            else
            {
              this->UART->statePort&=~0x02;     // Desactiva bandera de activacion de comunicaciones.
            }
        }
         else
        {
          this->UART->clearRxBuffPointer(this->UART); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
          this->UART->clearBufferRx();
          this->UART->statePort&=~0x02;
          this->UART->commTries=0;
          this->UART->strRxLen=0;
          this->UART->UartError&=~0x01;
          this->UART->extraTimeCounter=0;
        }

      }
      */
    }
  }
  return resp;
}
/****************************************************************************************************/
void CSatQ1000::setAlarm(char NRegister, char *Buffer, CRTC_3029_I2C *Reloj)
{
  int BufferRec[2];
  sscanf(Buffer, "%2d%2d", (int*)&BufferRec[0],(int*)&BufferRec[1]);
  Reloj->setAlarm(NRegister,(char)BufferRec[0], (char)BufferRec[1]);
}
/****************************************************************************************************/
unsigned long CSatQ1000::setLevelACKBuff(char* Buffer, unsigned char parameter, char statusCode)
{
  int x=0;
  Buffer[x++]=commandHeader;
  Buffer[x++]=linkLevelACKCommand;
  Buffer[x++]=0x07;
  Buffer[x++]=0x00;
  Buffer[x++]= ErrorTypeLevACK[statusCode];
  unsigned int l_CRC=this->calcFletcherCRC(Buffer,x+2);
  Buffer[x++]=l_CRC & 0x00FF;
  Buffer[x]=(l_CRC/0x0100)&0x00FF;
  return ++x;  // Devuelve el tamaño del comando a enviar vía serie
}
/****************************************************************************************************/
unsigned long CSatQ1000::setGetParameterBuff(char* Buffer, unsigned char parameter)
{
  int x=0;
  Buffer[x++]=commandHeader;
  Buffer[x++]=getParameterCommand;
  Buffer[x++]=0x08;
  Buffer[x++]=0x00;
  Buffer[x++]=this->retry;
  Buffer[x++]=parameter;
  unsigned int l_CRC=this->calcFletcherCRC(Buffer,x+2);
  Buffer[x++]=l_CRC & 0x00FF;
  Buffer[x]=(l_CRC/0x0100)&0x00FF;
  return ++x; // Devuelve el tamaño del comando a enviar vía serie
}
/****************************************************************************************************/
unsigned long CSatQ1000::setSetParameterBuff(char* Buffer, unsigned char parameter, char* value, char NBytesValue)
{
  int x=0,y;
  Buffer[x++]=commandHeader;
  Buffer[x++]=setParameterCommand;
  Buffer[x++]=9+NBytesValue;
  Buffer[x++]=0x00;
  Buffer[x++]=this->retry;
  Buffer[x++]=parameter;
  Buffer[x++]=NBytesValue;
  for(y=0;y<NBytesValue;y++)
  {
    Buffer[x++]=value[y];
  }
  unsigned int l_CRC=this->calcFletcherCRC(Buffer,x+2);
  Buffer[x++]=l_CRC & 0x00FF;
  Buffer[x]=(l_CRC/0x0100)&0x00FF;
  return ++x; // Devuelve el tamaño del comando a enviar vía serie
}
/****************************************************************************************************/
unsigned long CSatQ1000::setCommsCommandBuff(char* Buffer, char actionReq, unsigned char* valueByte, char idGateway)
{
  int x=0;
  Buffer[x++]=commandHeader;
  Buffer[x++]=commsCommCommand;
  Buffer[x++]=0x0D;
  Buffer[x++]=0x00;
  Buffer[x++]=this->retry;
  Buffer[x++]=actionReqCommComm[actionReq];
  Buffer[x++]=valueByte[0];
  Buffer[x++]=valueByte[1];
  Buffer[x++]=valueByte[2];
  Buffer[x++]=valueByte[3];
  Buffer[x++]=idGateway;
  unsigned int l_CRC=this->calcFletcherCRC(Buffer,x+2);
  Buffer[x++]=l_CRC & 0x00FF;
  Buffer[x]=(l_CRC/0x0100)&0x00FF;
  return ++x; // Devuelve el tamaño del comando a enviar vía serie
}
/****************************************************************************************************/
unsigned int CSatQ1000::calcFletcherCRC(char* Buffer, unsigned long length)
{
  char sum1=0x00;
  char sum2=0x00;
  int i;
  *(Buffer+length-1)=0;
  *(Buffer+length-2)=0;
  for(i=0;i<length;i++)
  {
    sum1+=*(Buffer+i);
    sum2+=sum1;
  }
  unsigned int result=(sum1-sum2)&0x00FF;
  result|=((sum2-(2*sum1))*0x0100)& 0xFF00;

  return result;
}
/****************************************************************************************************/
/****************************************************************************************************/
// Código adicional para evaluar mensajes del puerto local de comunicaciones RS-232 multiplexado con el puerto RS-232 satelital
/****************************************************************************************************/
int CSatQ1000::answertoCommand (char * bufferRx, char* bufferTx, char* flashSerialNumber, CLogger* memLogg, CEEPROM *memHandler, CRTC_3029_I2C* Reloj, CLcd *lcd)
{
int respLength=0;
int nBytes;
//char *l_data;
//  int *intPointer;

  strncpy(bufferTx,bufferRx,4);
  if(!(evalSN((char*)&bufferRx[5],flashSerialNumber)))
  {
     switch(bufferRx[0])
     {
     case 'H':
              switch(bufferRx[1])
              {
              case 'W':
                       switch(bufferRx[2])
                       {
                       case 'C':
                                switch(bufferRx[3])
                                {
                                case 'F':
                                         break;
                                case 'P': break;
                                case 'T': break;
                                default: break;
                                }
                                break;
                       case 'D':
                                switch(bufferRx[3])
                                {
                                case 'M': break;
                                case 'T':// Cambio de hora local del sistema
                                         // HWDT;1234567890;DDMMYYYYhhmmss;CRC;LFCR
                                         sscanf((char*)&bufferRx[16], "%2d%2d%4d%2d%2d%2d", (int*)&bufferRx[0],(int*)&bufferRx[2],(int*)&bufferRx[4],(int*)&bufferRx[6],(int*)&bufferRx[8],(int*)&bufferRx[10]);
                                         Reloj->hexToBcdRTC((int*)bufferRx);
                                         Reloj->setHour_Date(memHandler);
                                         Reloj->setRTCOnIntTime(memHandler,lcd,true,false);
                                         sprintf((char *)&bufferTx[4],":%s;","OK");
                                         respLength=8;
                                         break;
                                default: break;
                                }
                                break;

                       case 'F':
                                switch(bufferRx[3])
                                {
                                case 'S': // Verificar operacion.
                                         break;
                                default: break;
                                }
                                break;
                       case 'L':
                                switch(bufferRx[3])
                                {
                                case 'S':
                                         break;
                                default: break;
                                }
                                break;
                       case 'M':
                                switch(bufferRx[3])
                                {
                                case 'M':
                                         break;
                                case 'O':
                                         break;
                                default: break;
                                }
                                break;
                       case 'P':
                                switch(bufferRx[3])
                                {
                                case 'D':// Habilitar/Deshabilitar función de datalogger
                                         // HWPD;1234567890;E;CRC;LFCR  Habilita datalogger
                                         // HWPD;1234567890;D;CRC;LFCR  Deshabilita datalogger
                                         if(bufferRx[16]=='E')
                                         {memLogg->enableLogg(); respLength=10;}
                                         if(bufferRx[16]=='D')
                                         {memLogg->disableLogg(); respLength=10;}
                                         if(respLength)
                                         {
                                            sprintf((char *)&bufferTx[4],":%c;%s;",bufferRx[16],"OK");
                                         }
                                         break;
                                default: break;
                                }
                                break;
                       case 'R':
                                switch(bufferRx[3])
                                {
                                case 'P':   // Comando psrs reinicializar el puntero de escritura de datalogger local (para ser Tx via Puerto serie local)
                                         //memLogg->setPointerWrite(0x000000); // Puntero de inicio de memoria del Logger interno
                                         sprintf((char*)&bufferTx[4],":%s;","OK");
                                         respLength=8;
                                         break;
                                default: break;
                                }
                                break;
                       case 'S':
                                switch(bufferRx[3])
                                {
                                case 'D':// Cambio de horarios para reportes vía satelital
                                         // HWSD;1234567890;N;HHMM;HHMM;HHMM;CRC;LFCR
                                        sscanf((char*)&bufferRx[16], "%d", (int*)&bufferRx[0]);
                                        Reloj->setNAlarms(*(int*)&bufferRx[0]);
                                        for(int x=0;x<Reloj->getNAlarms();x++)
                                        {
                                          this->setAlarm(x, (char*)&bufferRx[(5*x)+18], Reloj);
                                        }
                                        sprintf((char*)&bufferTx[4],":%s;","OK");
                                        respLength=8;
                                        break;
                                case 'P': // Cambio de tiempo de muestreo para datalogger
                                          // HWSP;1234567890;HHMMSS;CRC;LFCR
                                         sscanf((char*)&bufferRx[16], "%2ld%2ld%2ld", (unsigned long int*)&bufferRx[0],(unsigned long int*)&bufferRx[4],(unsigned long int*)&bufferRx[8]);
                                         //memLogg->setDefSampleTime((*(unsigned long int*)&bufferRx[0]*3600)+(*(unsigned long int*)&bufferRx[4]*60)+(*(unsigned long int*)&bufferRx[8]));
                                         sprintf((char *)&bufferTx[4],":%s;","OK");
                                         respLength=8;
                                         break;
                                case 'U':
                                          break;
                                default: break;
                                }
                                break;
                       case 'U':
                                switch(bufferRx[3])
                                {
                                case 'M':
                                         break;
                                default: break;
                                }
                                break;
                       case 'V':
                                switch(bufferRx[3])
                                {
                                case 'L': // Verificar operacion.
                                         break;
                                case 'P': // Verificar operacion.
                                         break;
                                default: break;
                                }
                                break;
                       default: break;
                       }
                       break;
              case 'R':
                       switch(bufferRx[2])
                       {
                       case 'C':
                                switch(bufferRx[3])
                                {
                                case 'F':
                                         break;
                                case 'I':// Devuelve el número de serie del sistema (confirmación del número de serie)
                                         // HRCI;1234567890;CRC;LFCR
                                         bufferTx[4]=':';
                                         strncpy((char *)&bufferTx[5],flashSerialNumber,10);
                                         bufferTx[15]=';';
                                         respLength=16;
                                         break;
                                case 'P': break;
                                case 'T': break;
                                default: break;
                                }
                                break;
                       case 'D':
                                switch(bufferRx[3])
                                {
                                case 'T':
                                default: break;
                                }
                                break;
                       case 'F':
                                switch(bufferRx[3])
                                {
                                case 'S':break;
                                case 'L':
                                         break;
                                case 'V':
                                         break;
                                default: break;
                                }
                                break;
                       case 'I':
                                switch(bufferRx[3])
                                {
                                case 'D':
                                         break;
                                default: break;
                                }
                                break;
                       case 'L':
                                switch(bufferRx[3])
                                {
                                case 'B': // Lectura de un bloque de memoria del datalogger
                                          // HRLB;1234567890;BBBB;CRC;LFCR
                                         //if(flowmeter->flagsEEPROM&0x08)  // Verifica que la bandera de requerimiento de información de la memoria EEPROM se encuentre Activada.
                                         //{
                                           sscanf((char*)&bufferRx[16], "%4d", &nBytes);  // Obtiene numero de bloque a leer
                                           if(nBytes)
                                           { // Código para enviar el bloque (128 bytes de memoria) de datos definido (1 a 9999).
                                              bufferTx[4]=':';
                                              bufferTx[5]=bufferRx[16];bufferTx[6]=bufferRx[17];bufferTx[7]=bufferRx[18];bufferTx[8]=bufferRx[19];bufferTx[9]=';';

                                              /*memLogg->readLoggDataByBlk((char*)&bufferTx[10], memHandler, 2*nBytes-1); // Para una memoria de 128 KB
                                              nBytes++;
                                              memLogg->readLoggDataByBlk((char*)&bufferTx[138], memHandler, 2*nBytes); // Para una memoria de 128 KB
                                              */

                                              memLogg->readLoggDataByBlk((char*)bufferRx, memHandler, nBytes); // Para una memoria de 128 KB

                                              for(nBytes=0; nBytes<128;nBytes++) // Leer 1 bloques de 128 bytes
                                              {
                                                 sprintf((char *)&bufferTx[nBytes*2+10],"%.2X",bufferRx[nBytes]);
                                              }
                                              bufferTx[266]=';';
                                              respLength=267;
                                           }
/*                                           else
                                           { // Código para finalizar la sesión de bajar la información.
                                              if(flowmeter->flagsEEPROM&0x10)   // Evalua si bandera de logger activado anterior se encuentra activada.
                                              {
                                                 flowmeter->flagsEEPROM&=~0x10;     //Desactiva bandera de logger activado anterior.
                                                 changeDLMode(flowmeter);          // Activa el logger.
                                              }
                                              setClock(&flowmeter->countSTLogg, 0x01, 0x01, 2000, 0x00, 0x00, 0x00);// Borra el contador de recolección.
                                              resetWRRegEEPROM(flowmeter); // Reinicia el apuntador de escritura en EEPROM.
                                              flowmeter->flagsEEPROM&=~0x08;    // Desactiva bandera de requerimiento de información de la memoria EEPROM.
                                              sprintf((char *)&bufferTx[4], ":%.2s;","OK");
                                              respLength=8;
                                           }*/
                                         //} */
                                         break;
                                default: break;
                                }
                                break;
                       case 'M':
                                switch(bufferRx[3])
                                {
                                case 'M':
                                         break;
                                case 'O':// Lectura para obtener la cabecera de inivio de lectura de datalogger
                                         // HRMO;1234567890;LFCR
                                         //l_data=(char*)memLogg->getPointerWrite();
                                         //sprintf((char *)&bufferTx[4],":%.2X%.2X%.2X%.2X;",*(char*)l_data,*(char*)(l_data+1),*(char*)(l_data+2),*(char*)(l_data+3));
                                         //l_data=(char*)memLogg->getDefSampleTime();
                                         //sprintf((char *)&bufferTx[14],"%.2X%.2X%.2X%.2X;",*(char*)l_data,*(char*)(l_data+1),*(char*)(l_data+2),*(char*)(l_data+3));
                                         respLength=23;
                                         break;
                                default: break;
                                }
                                break;
                       case 'N':
                                switch(bufferRx[3])
                                {
                                case 'C':
                                         break;
                                default: break;
                                }
                                break;
                       case 'P':
                                switch(bufferRx[3])
                                {
                                case 'D':
                                         break;
                                case 'O':
                                         break;
                                case 'T':
                                         break;
                                default: break;
                                }
                                break;
                       case 'S':
                                switch(bufferRx[3])
                                {
                                case 'F':
                                         break;
                                case 'P':
                                         break;
                                default: break;
                                }
                                break;
                        case 'U':
                                switch(bufferRx[3])
                                {
                                case 'M':
                                         break;
                                default: break;
                                }
                                break;
                       case 'V':
                                switch(bufferRx[3])
                                {
                                case 'L':
                                         break;
                                case 'P': // Leer volumen por pulso
                                         break;
                                default: break;
                                }
                                break;
                       case 'W':
                                switch(bufferRx[3])
                                {
                                case 'P':
                                         break;
                                default: break;
                                }
                                break;
                       default: break;
                       }
                       break;
              default: break;
              }
              break;

     default:break;
     }
  }
  else
  {
     switch(bufferRx[0])
     {
     case 'H':
              switch(bufferRx[1])
              {
              case 'R':
                       switch(bufferRx[2])
                       {
                       case 'I':
                                switch(bufferRx[3])
                                {
                                case 'D':
                                         // Devuelve el número de serie del sistema (cuando no se conoce el número de serie)
                                         // HRID;1234567890;CRC;LFCR
                                         bufferTx[4]=':';
                                         strncpy((char *)&bufferTx[5],flashSerialNumber,10);
                                         bufferTx[15]=';';
                                         respLength=16;
                                         break;
                                default: break;
                                }
                                break;
                       default: break;
                       }
                       break;
              default: break;
              }
              break;
     default: break;
     }
  }

  if (respLength)
  {
     sprintf((char *)&bufferTx[respLength], "%.2X\r\n",calcCRC((char *)&bufferTx[0],respLength));
     respLength+=4;
  }

   return  respLength;
}
/****************************************************************************************************/
/**********************************************************************************/
char CSatQ1000::evalSN(char * IDBufferRx, char*IDBuffer)   //devuelve 0 si el ID del medidor es igual que el recibido en el comando
{
   char resp=0x00;
   for(int x=0;x<10;x++)
   {
      if(IDBufferRx[x] != IDBuffer[x])
      {resp=0x01; break;}
   }
   return resp;
}
/**********************************************************************************/
char CSatQ1000::calcCRC(char* xBuffer, int length)
{
  int CRC=0;
  for(int x=0; x<length; x++)
     CRC^=*xBuffer++;
  return CRC+1;
}
/**********************************************************************************/
char CSatQ1000::getCRCbuffer(char* xBuffer, int length)
{
  length-=4;
  return (asctobcd(xBuffer[length]) * 0x10) | asctobcd(xBuffer[length+1]);
}
/**********************************************************************************/
char CSatQ1000::asctobcd(char asc)
{
  if (asc>=0x30 && asc<=0x39)
    asc-=0x30;
  else
    asc-=0x37;

  return asc&0x0F;
}
/**********************************************************************************/
unsigned long CSatQ1000::SC_OriginatedDefMsg(char* Buffer, CLogger* mLoggerSAT, CItzyFlow* mItzyFlow, CEEPROM *memHandler, char qtyMed)
{
  // Establecer en loggerSAt un límite de 0x300 datos
  int x=0, x_Acum=0;
  int y;
  char indexMed,reg_temp1;
  int p_startBlk,l_NBlocks;
  Buffer[x++]=commandHeader;
  Buffer[x++]=0x07; // # de comando
  x+=2;
  Buffer[x++]=this->retry;
  if(this->mha==0x00)
  {this->mha=0x01;}
  Buffer[x++]=this->mha++;

  x_Acum+=x;

  for(indexMed=0;indexMed<*mItzyFlow->getNMedidores();indexMed++)
{
  x=0;

  switch(indexMed)
  {
  case 1: p_startBlk=3077; //0x060280;
          break;
  case 2: p_startBlk=3081; //0x060480;
          break;
  case 3: p_startBlk=3085; //0x060680;
          break;
  case 0: p_startBlk=3073; //0x060080;
  default: break;
  }

  l_NBlocks=(*(mItzyFlow->Flowmeter[indexMed]->getPointerWrite())-1)/128;
  if((l_NBlocks-(p_startBlk-2))>0)
  {
    for(;p_startBlk<=l_NBlocks+1;p_startBlk++)
    {
      mLoggerSAT->readLoggDataByBlk(&Buffer[x_Acum+x], memHandler, p_startBlk); // Solo para una memoria de 128 KB
      x+=128;
    }

    sprintf((char*)&Buffer[17],"%.3d",*mLoggerSAT->getPNDataBCD());
    Buffer[20]=',';

    y=(*(mItzyFlow->Flowmeter[indexMed]->getPointerWrite()) & 0x0000007F);
    if(y)
    {
      x-=(128-y);
    }
    x_Acum+=x;

    if(indexMed==0) // Evalua si se obtuvieron datos de los primeros regitros del medidor 1 para obtener la cantidad de registros.
    {
      x-=11; // 11 corresponde a la cantidad de bytes de la cabecera que se inserta al inicio de los registros a enviar (1er medidor).
      reg_temp1=*mItzyFlow->Flowmeter[0]->getIDVariable();
      switch(reg_temp1) // Evalua el ID de tipo de variables leeidas del medidor 1
      {
      case 4:
      case 1:  reg_temp1=7; break;
      case 5:
      case 2:  reg_temp1=5; break;
      case 6:
      case 3:  reg_temp1=3; break;
      case 0:
      default: reg_temp1=9; break;
      }
      reg_temp1=(x/reg_temp1);
      Buffer[16]=reg_temp1; // Posición de byte identificador de cantidad de resgitros enviados.
    }
  }
  else
  {
    // Código para ensamblar la trama satelital para error de enlace de telemetría con Scadapack
    mLoggerSAT->readLoggDataByBlk(&Buffer[x_Acum], memHandler, 3073); // Solo para una memoria de 128 KB
    x_Acum+=8;
    break;
    //return 0;
  }

  }

  if(x_Acum>6)
  {
//    Establece la cantidad de datos a enviar
    Buffer[2]=(x_Acum+2)&0x00FF;         // LSB Length
    Buffer[3]=((x_Acum+2)/0x100)&0x00FF; // MSB Length


    // Calculo de CRC
    unsigned int l_CRC=this->calcFletcherCRC(Buffer,x_Acum+2);
    Buffer[x_Acum++]=l_CRC & 0x00FF;
    Buffer[x_Acum]=(l_CRC/0x0100)&0x00FF;

    mItzyFlow->restartWRPointers();
    mLoggerSAT->clearNData();

    return ++x_Acum; // Devuelve el tamaño del comando a enviar vía serie
  }
  else
  {
    return 0;
  }

}
/**********************************************************************************/
unsigned long CSatQ1000::linkLevelAcknowledgment(char* Buffer, unsigned char statusCode)
{
  int x=0;
  Buffer[x++]=commandHeader;
  Buffer[x++]=0x01;
  Buffer[x++]=0x07;
  Buffer[x++]=0x00;
  Buffer[x++]=statusCode;
  unsigned int l_CRC=this->calcFletcherCRC(Buffer,x+2);
  Buffer[x++]=l_CRC & 0x00FF;
  Buffer[x]=(l_CRC/0x0100)&0x00FF;
  return ++x; // Devuelve el tamaño del comando a enviar vía serie
}
/**********************************************************************************/

const int T_DaysInFourYears [5] ={0,366,731,1096,1461};
// Cantidad de dias acumulados por mes año normal (365 días)
const int T_DaysInNormMonth [13] ={0,31,59,90,120,151,181,212,243,273,304,334,365};
// Cantidad de dias acumulados por mes año bisciesto (366 días)
const int T_DaysInBiscMonth [13] ={0,31,60,91,121,152,182,213,244,274,305,335,366};

/**********************************************************************************/

void CSatQ1000::calcDateModemQ1000(unsigned int nWeeks, long  nSeconds, CRTC_3029_I2C* clkRTC, CEEPROM* Mem, CLcd* m_LCD)
{

int Buffer[6];

long totalSec=nSeconds;

totalSec+=(clkRTC->timeZone*3600); // una hora contiene 3600 segundos

if(totalSec>=604800) // una semana contiene 604800 segundos, si es que timezone>0
{
  nSeconds=totalSec-604800;
  nWeeks++;
}
else if(totalSec<0)
{
  nSeconds=604800+totalSec;
  nWeeks--;
}
else
{
  nSeconds=totalSec;
}

unsigned long l_days=6;
int x;
int sum_años=1980;
int  l_month=0;
char l_dayweek=0;

if(nWeeks)
{

  l_days=7*(nWeeks);

// Cada 28 años se repiten las fechas por lo tanto 28 años es igual a 10227 dias (contando años bisciestos).
  ldiv_t prev_años= div(l_days,(long)10227);

  sum_años=prev_años.quot*28;
  sum_años+=1980;

// 4 años contienen 1461 días (incluyendo el año bisciesto).
  prev_años=div(prev_años.rem,(long)1461);

  sum_años+=(prev_años.quot*4);

  for(x=0;x<4;x++)
  {
    if(prev_años.rem>T_DaysInFourYears[x] && prev_años.rem<=T_DaysInFourYears[x+1])
    {
      break;
    }
  }
  sum_años+=x;
  l_days=(prev_años.rem-T_DaysInFourYears[x]);
  if(x)
  {
    for(x=0;x<12;x++)
    {
      if(l_days>T_DaysInNormMonth[x] && l_days<=T_DaysInNormMonth[x+1])
      {
        break;
      }
    }
    l_days-=T_DaysInNormMonth[x];
    l_dayweek=this->calcDay_Hours(nSeconds, clkRTC);
    l_days+=(6 + l_dayweek);
    if(l_days>T_DaysInNormMonth[x+1]-T_DaysInNormMonth[x])
    {
      l_days-=(T_DaysInNormMonth[x+1]-T_DaysInNormMonth[x]);
      x++;
    }
  }
  else
  {
    for(x=0;x<12;x++)
    {
      if(l_days>T_DaysInBiscMonth[x] && l_days<=T_DaysInBiscMonth[x+1])
      {
        break;
      }
    }
    l_days-=T_DaysInBiscMonth[x];
    l_dayweek=this->calcDay_Hours(nSeconds,clkRTC);
    l_days+=(6 + l_dayweek);
    if(l_days>T_DaysInBiscMonth[x+1]-T_DaysInBiscMonth[x])
    {
      l_days-=(T_DaysInBiscMonth[x+1]-T_DaysInBiscMonth[x]);
      x++;
    }
  }

  l_month=x;  // 0=enero, 11=Diciembre

}

if(l_month>=12)
{
  l_month=0;
  sum_años++;
}

  clkRTC->day=l_days;
  clkRTC->year=sum_años;
  clkRTC->month=l_month+1;

  Buffer[0]=clkRTC->day & 0x00FF;
  Buffer[1]=clkRTC->month & 0x00FF;
  Buffer[2]=clkRTC->year;
  Buffer[3]=clkRTC->hour & 0x00FF;
  Buffer[4]=clkRTC->min & 0x00FF;
  Buffer[5]=clkRTC->seg & 0x00FF;

  clkRTC->hexToBcdRTC(&Buffer[0]);
  clkRTC->setHour_Date(Mem);
  clkRTC->setRTCOnIntTime(Mem, m_LCD,true,false);

}
/****************************************************************************/
int CSatQ1000::calcDay_Hours(long nSeconds, CRTC_3029_I2C* clkRTC)
{
  ldiv_t l_reg = div(nSeconds,(long)60);
  clkRTC->seg = l_reg.rem;
  l_reg = div(l_reg.quot,(long)60);
  clkRTC->min = l_reg.rem;
   l_reg = div(l_reg.quot,(long)24);
  clkRTC->hour = l_reg.rem;

  return l_reg.quot;
}
/****************************************************************************/
void CSatQ1000::disableSendingReport()
{
  this->UART->statePort&=~0x04;
}
/****************************************************************************/
void CSatQ1000::enableSendingReport()
{
  this->UART->statePort|=0x04;
}
/****************************************************************************/
bool CSatQ1000::isEnableSendingReport()
{
  if(this->UART->statePort&0x04)
  {
    return true;
  }
  return false;
}
/****************************************************************************/
char* CSatQ1000::getAddrFlagsUARTstate()
{
  return &this->UART->statePort;
}
/****************************************************************************/
bool CSatQ1000::evalAndExecuteReqRemote(char* Buffer)
{
  char* p_Char;
  bool resp=false;

  if(Buffer[6])
  {
    p_Char=strstr(&Buffer[8],"HCommand");
    if(p_Char)
    {
      p_Char+=10;
      p_Char=strstr(p_Char,"H");
      if(p_Char)
      {
        p_Char++;
        switch(*p_Char)
        {
        case 'W':
                 p_Char++;
                 switch(*p_Char)
                 {
                 case 'R':
                          p_Char++;
                          switch(*p_Char)
                          {
                          case 'D':
                                   p_Char+=2;
                                   if(*p_Char == 'E')
                                   {
                                     this->enableSendingReport();
                                     resp=true;
                                   }
                                   if(*p_Char == 'D')
                                   {
                                     this->disableSendingReport();
                                     resp=true;
                                   }
                                   break;
                          default: break;
                          }
                          break;
                 default: break;
                 }
                 break;
        default: break;

        }

      }
     }
  }
return resp;
}
/****************************************************************************/
void CSatQ1000::OnModemSat()
{
  P1DIR&=~ON_OFF_SAT; // Establece el puerto como entrada (Envia a 1)
//  P1OUT|=ON_OFF_SAT;
}
/****************************************************************************/
void CSatQ1000::OffModemSat()
{
  P1DIR|=ON_OFF_SAT;  // Establece el puerto como salida (Envia a 0)
//  P1OUT&=~ON_OFF_SAT;
}
/****************************************************************************/
bool CSatQ1000::isModemSatOn()
{
 // if(P1IN & ON_OFF_SAT)
  if(!(P1DIR & ON_OFF_SAT))
  { return true;}
  return false;
}
/****************************************************************************/




