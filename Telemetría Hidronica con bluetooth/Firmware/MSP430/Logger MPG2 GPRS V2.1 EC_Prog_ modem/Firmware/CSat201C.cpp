#include "CDevice.h"
//#include "string.h"
#include "string"

const char Sat201CCommand[9][9]={
                            "CA0007\r",                                                 //0 auto select D+ mode start up. + Automatic switching + follow best/switch.
                            "CB01\r",                                                   //1 AORW- Atlantic Ocean Region West
                            "CE07\r",                                                   //2 Serial buffer enabled + Terminal power enabled + LED enabled + ADC disabled
                            "GY00001\r",                                                //3 1min resinchronisation UTC
                            "MO0\r",                                                    //4 Enable output of the mf messages automatically
                            "MT?\r",                                                    //5 Report Transmitter status
                            "MR?\r",                                                    //6 Report Transmitter status
                            "MM L ",                                                    //7 Send Large Message (22 Characters)
                            "TT?\r",                                                    //8 Get Current Time and Date
                          };

/****************************************************************************************************/
const char ResponsesSat201C[12][4]=
{
  "ca ",  // 0
  "cb ",  // 1
  "ce ",  // 2
  "gy ",  // 3
  "mo ",  // 4
  "mt ",  // 5
  "mr ",  // 6
  "mm ",  // 7
  "tt ",  // 8
};
/*********************************************************************/
CSat201C::CSat201C()
{
  this->stateSat201C=0x00;
  this->commCounter=0x00;
  this->nReportQty=0x01;
  this->nReport=0x00;
  UART=new CUart0();
}
/*********************************************************************/
CSat201C::~CSat201C()
{
}
/*********************************************************************/
/****************************************************************************************************/
// Implementación de Funciones.
/****************************************************************************************************/
void CSat201C::configSat201C()
{
}
/****************************************************************************************************/
void CSat201C::turnOnSat201C(void)
{

}
/****************************************************************************************************/
void CSat201C::turnOffSat201C(void)
{

}
/****************************************************************************************************/
void CSat201C::sendFrameToSat201C(int Command)
{
     int mylen=0;
     sprintf((char *)&this->UART->Buffer_Tx[0],"%s",Sat201CCommand[Command]);
     mylen=strlen((char *)&this->UART->Buffer_Tx[0]);
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
void CSat201C::sendReportSat201C(CPanametrics *pflowmeter, int Command)
{
  char *myData;
  sprintf((char *)&this->UART->Buffer_Tx[0],"%s",Sat201CCommand[Command]);

  switch(this->nReport)
  {
   case 1:  // Reporte de Velocidad, Error e Intensidad de Señales
/*  char *getUnitsFlowSpeed();
  unsigned int *getSSUp();
  unsigned int *getSSDown();
     */
          sprintf((char *)&this->UART->Buffer_Tx[5],"%.2X",0x91); // 4 bits tipo de producto ; 4bits tipo de mensaje

          myData=(char*)pflowmeter->getFlowSpeed();
          sprintf((char *)&this->UART->Buffer_Tx[7],"%.2X%.2X%.2X%.2X",*(myData+3),*(myData+2),*(myData+1),*myData);
          myData=pflowmeter->getUnitsFlowSpeed();
          //*myData>>=1;
          sprintf((char *)&this->UART->Buffer_Tx[15],"%.1X",*myData);

          myData=(char*)pflowmeter->getIDError();
          sprintf((char *)&this->UART->Buffer_Tx[16],"%.1X",*myData);

          myData=(char*)pflowmeter->getSSUp();
          sprintf((char *)&this->UART->Buffer_Tx[17],"%.1X%.2X",(*(myData+1))&0x0F,*myData);

          myData=(char*)pflowmeter->getSSDown();
          sprintf((char *)&this->UART->Buffer_Tx[20],"%.1X%.2X",(*(myData+1))&0x0F,*myData);


          sprintf((char *)&this->UART->Buffer_Tx[23],"%s","0000\r"); // Se completan 22 caracteres a pesar de que solo llegan 21 al servidor
          break;
   case 0:
   default: // Reporte de Gasto y Volumen Fwd+

          sprintf((char *)&this->UART->Buffer_Tx[5],"%.2X",0x90); // 4 bits tipo de producto ; 4bits tipo de mensaje

          myData=(char*)pflowmeter->getFlowrate();
          sprintf((char *)&this->UART->Buffer_Tx[7],"%.2X%.2X%.2X%.2X",*(myData+3),*(myData+2),*(myData+1),*myData);
          myData=pflowmeter->getUnitsFlowrate();
          sprintf((char *)&this->UART->Buffer_Tx[15],"%.2X",*myData);

          myData=(char*)pflowmeter->getTotalizerFwd();
          sprintf((char *)&this->UART->Buffer_Tx[17],"%.2X%.2X%.2X%.2X",*(myData+3),*(myData+2),*(myData+1),*myData);
          myData=pflowmeter->getUnitsTotalizerFwd();
          sprintf((char *)&this->UART->Buffer_Tx[25],"%.1X%s",*myData,"0\r"); // Se completan 22 caracteres a pesar de que solo llegan 21 al servidor
          break;
  }

  int mylen=0;
  mylen=strlen((char *)&this->UART->Buffer_Tx[0]);
  this->UART->startTx((char *)&this->UART->Buffer_Tx[0],mylen); // Envía la cadena de datos por el puerto serie.
}
/****************************************************************************************************/
void CSat201C::setupSat201C()
{
  this->commCounter=0;
  this->endCommandIndex=4;
  sendFrameToSat201C(this->commCounter);
}
/****************************************************************************************************/
void CSat201C::getTimeDateSat201C()
{
  this->commCounter=8;
  this->endCommandIndex=8;
  sendFrameToSat201C(this->commCounter);
}
/****************************************************************************************************/
void CSat201C::connectSat201C()
{
  turnOnSat201C();
  this->commCounter=4;
  this->endCommandIndex=7;
  sendFrameToSat201C(this->commCounter);
}
/****************************************************************************************************/
//  response:
//  0: No valida la respuesta (requiere time out)
//  1: Valida la respuesta
//  2: Requiere timeout y retransimisión de la cadena
//  3: Valida la respuesta de comado de Rx remoto (Del centro de datos) y envía el ID que identifica el cambio de la variable del sistema solicitada.
//  4: Valida la respuesta de comando de Rx remoto (del centro de datos) y envía el ID para respuesta de datos vía satelita.

unsigned char CSat201C::evalSat201CResponse(int *commandSent, unsigned char* BufferRx, int* RxLength, CRTC_3029_I2C *myClock)
{
  unsigned char response=0x00;
  char *myStr=0x0000;

  char * mypointer=0x0000;
//  int x;
  char Buffer[4];

  myStr=(char*)ResponsesSat201C[*commandSent];
  mypointer=(char *)strstr((const char *)&BufferRx[0],myStr);

  if(mypointer) // Respuesta adecuada?
  {
     switch(*commandSent)
     {
     case 5:// Evaluación de la bandera de Tx
            mypointer+=9;
            sscanf(mypointer, "%1d%1d", (int*)&Buffer[0],(int*)&Buffer[2]);
            Buffer[0]=((Buffer[0]&0x0F)*0x10)|(Buffer[2]&0x0F);

            if(Buffer[0]&0xE0)
            {
              response=0x02; //Requiere timeout y retransmisión de la cadena a enviar via satelital
              this->nReport--;
              if(this->nReport<0)
              {
                this->nReport=0;
              }
              break;
            }
            if(Buffer[0]&0x18)
            {
              response=0x00; // Requiere de timeout
              break;
            }
            if(Buffer[0]&0x03)
            {
              response =0x00; // requiere de timeout
             break;
            }
            if(Buffer[0]&0x04)
            {
              response=0x01;  // Banderas válidas para Transmitir
              break;
            }


     case 6:
            response =0x01; break;
     case 7:
            response=0x01;  break;

     default:response=0x01;
             break;
     }

/*      if(*commandSent!=8)
      {
        for(x=0;x<*RxLength;x++)    // Borra el contenido del buffer de Rx
        {
          BufferRx[x]=0x00;
        }
      }
*/
  }
  else
  { // codigo para detectar mensajes remotos (del centro de datos).

    mypointer=(char *)strstr((const char *)&BufferRx[0],"mf");
    mypointer+=25;
    switch(*mypointer)
    {
    case 'H':mypointer++;
             switch(*mypointer)
             {
             case 'R':mypointer++;
                      switch(*mypointer)
                      {
                      case 'D':mypointer++;
                               switch(*mypointer)
                               {
                               case 'T':
                                        switch(*(mypointer+3))
                                        {
                                        case 'V':response=0x04;  break;// respuesta solo para requerimientos de información via externa (de la central de datos).
                                        default:  break;
                                        }
                                        break;
                               default: break;
                               }
                               break;
                      default: break;
                      }
                      break;

             case 'W':mypointer++;
                      switch(*mypointer)
                      {
                      case 'S':mypointer++;
                               switch(*mypointer)
                               {
                               case 'D':  // Cambio de cantidad de horarios y hora de cada uno.
                                        sscanf(mypointer+3, "%1d", (int*)&Buffer[0]);
                                        myClock->setNAlarms(*(int*)&Buffer[0]);
                                        for(int x=0;x<myClock->getNAlarms();x++)
                                        this->setAlarm(x, (mypointer+(5*(x+1))), myClock);
                                        response=0x03;  // respuesta solo para requerimientos via externa (de la central de datos).
                                        break;
                               case 'O':  // Cambio de habilitación y tipo de reportes
                                        switch(*(mypointer+3))
                                        {
                                        case 'E': myClock->regEnableOp=true; break; //Habilita la funcionalidad de evaluación de reporte por horarios.
                                        case 'D':
                                        default:  myClock->regEnableOp=false; break; //Deshabilita la funcionalida de evaluación de reporte por horarios.
                                        }
                                        switch(*(mypointer+5))
                                        {
                                        case 'T': this->nReportQty=0x02; break; //Cambia tipo de reportes a Total (2 mensajes)
                                        case 'S': this->nReportQty=0x01;        // Cambia tipo de reportes a Standard (1 mensaje)
                                        default:   break; //Deshabilita la funcionalida de evaluación de reporte por horarios.
                                        }
                                        response=0x03;  // respuesta solo para requerimientos via externa (de la central de datos).
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

  return response;
}
/****************************************************************************************************/
void CSat201C::changeDateGPS(CRTC_3029_I2C *myClock, CLcd *myLCD)
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

  myClock->setClockWLCD(myLCD,myClock->day, myClock->month, myClock->year, myClock->hour, myClock->min, myClock->seg, true,false);
}
/****************************************************************************************************/
void CSat201C::setKReport(char kreport)
{
  kreport++;
  this->nReportQty=kreport;
}
/****************************************************************************************************/
int CSat201C::getKReport()
{
  return this->nReportQty;
}
/****************************************************************************************************/
int CSat201C::timeoutTxRx(CPanametrics *myFlowmeter,CRTC_3029_I2C *myClock, CLcd *myLCD)
{
  int resp=0;
  int IDEval;
  if(this->UART->statePort&0x02)  // Comunicaciones Tx o Rx del módulo GSM?
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
      if(this->UART->strRxLen)// Se recibió una cadena de datos.
      {
        IDEval=this->evalSat201CResponse(&this->commCounter, this->UART->Buffer_Rx, &this->UART->strRxLen, myClock);
        if (IDEval==0x01) // Se recibió la respuesta de manera correcta del módulo Satelital?
        {
          // Adicionar lineas de configuración de registros vía puerto serie del modulo satelital.
          switch(this->commCounter)
          {
          case 8:
            this->changeDateGPS(myClock, myLCD);
            break;
          default: break;
          }
          // Fin lineas de configuración de registros via puerto serie del modulo satelital

          this->UART->extraTimeCounter=0;
          this->UART->commTries=0;
          this->UART->clearRxBuffPointer(this->UART); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
          this->UART->strRxLen=0;
          this->UART->clearBufferRx();

          if(this->commCounter<this->endCommandIndex)
          {
                this->commCounter++;
                if(this->commCounter!=7)
                {
                  this->sendFrameToSat201C(this->commCounter);
                }
                else
                {
                  this->sendReportSat201C(myFlowmeter, this->commCounter);
                }
          }
          else
          {
            this->stateSat201C&=~0x01;
            switch(this->commCounter)
            {
               case 8: resp=1; break;
               default: resp=2; break;
            }

          }
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
                resp=3; // 1
              }
              else // Retransmite último comando solicitado.
              {
                if(this->commCounter!=7)
                {
                  this->sendFrameToSat201C(this->commCounter);
                }
                else
                {this->sendReportSat201C(myFlowmeter, this->commCounter);}
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
            //Finaliza retransimisiones a GPRS
            this->UART->commTries=0;
            this->UART->strRxLen=0;
            this->UART->UartError&=~0x01;
            resp=1; // 1
          }
          else // Retransmite último comando solicitado.
          {
             if(this->commCounter!=7)
              {
                 this->sendFrameToSat201C(this->commCounter);
              }
              else
              {
                this->sendReportSat201C(myFlowmeter, this->commCounter);
              }
          }
        }
      }

    }
  }
  return resp;
}
/****************************************************************************************************/
void CSat201C::setAlarm(char NRegister, char *Buffer, CRTC_3029_I2C *Reloj)
{
  int BufferRec[2];
//  NRegister--;
  sscanf(Buffer, "%2d%2d", (int*)&BufferRec[0],(int*)&BufferRec[1]);
  Reloj->setAlarm(NRegister,(char)BufferRec[0], (char)BufferRec[1]);
}
/****************************************************************************************************/





