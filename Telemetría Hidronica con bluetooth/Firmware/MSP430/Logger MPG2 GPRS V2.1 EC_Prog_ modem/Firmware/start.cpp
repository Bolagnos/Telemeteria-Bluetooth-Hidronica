/**********************************************************************************************/
/* Hidronica S.A. de C.V.                                                                     */
/* Programador: Ing. Fernando Bolaños G.                                                      */
/* Fecha de desarrollo: December 2016                                                        */
/* Programa: MPG2 GPRS Comms                                                                  */
/* Version: 2.0                                                                               */
/* Comunica con modem Quectel UC20 o EC20 y medidor MPG2                                   */
/* Almacena registros de gasto y volumen en memoria EEPROM                                    */
/**********************************************************************************************/
/* Stack Size: 1000 Bytes                                                                     */
/* Heap size: 7000 Bytes                                                                      */
/* Double Floating-point 64 bits                                                              */
/**********************************************************************************************/

/**********************************************************************************************/
/*                                 Inicio de Programa                                         */
/**********************************************************************************************/
#include "CModule.h"
#include "stdio.h"
#include "math.h"

__no_init CModule *mySystem @ 0x1100;

//char StrVal[]={"HWSD;1701080108;2;1213;1516;0000;"};

/*const  char m_Buffer[]= {0x00,0x05,0x14,0x0c,0x00,0x00,0x00,0x17,0x02,0x61,0x06,0xeb,0x70};
const char m_Buffer1[]={0x05,0x14,0x0e,0x00,0x00,0x00,0x18,0x04,0x0a,0x51,0x00,0x00,0x6d,0xb4,0x01};
*/

int main(void)
{

   mySystem=new CModule();

   mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);

   _EINT();

   mySystem->VLevelBatt->getVoltageBatt();

   if(mySystem->VLevelBatt->getVpot()>=Lev_Disch_Comm)
   {
      mySystem->RGBLed->onOnlyGreen();
   }
   else
   {
      mySystem->RGBLed->onOnlyRed();
   }

   mySystem->Reloj->stopReloj();


   mySystem->ModemGPRS->counterTimeout=0;
   mySystem->ModemGPRS->timerReset->EnableTBCCTL0_IE();
   mySystem->ModemGPRS->OnModemGPRS();
   mySystem->MPG2->set_ID_Rep(0x31);
   mySystem->MPG2->requestData=true;
   mySystem->ModemGPRS->setupGPRS();

   while(true)
   {
      LPM3;             // Low Power Mode 3

      if(mySystem->VLevelBatt->isFlagReq())
      {
        mySystem->VLevelBatt->clearFlagReq();
        mySystem->VLevelBatt->getVoltageBatt();
      }

      if(mySystem->m_btnProg->isProgFlagActivated())
      {
        if(!mySystem->m_btnProg->isButtonPressed()) // Cumplió 2 minutos?
        {// Desabilita el dispsitivo RS-232 por completo
          mySystem->ModemGPRS->UART->off_RS_232();
          mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
          mySystem->m_btnProg->clearProgFlag();
        }
        else
        {
          mySystem->ModemGPRS->UART->on_RS_232();
        }
      }
/*      if(!mySystem->ModemGPRS->isModemGPRSOn())
      {
        P5OUT&=~BIT7;    // Apaga LED en el PCB de estado del modem GSM
      }
      else
      {
        P5OUT|=BIT7;    // Prende LED en el PCB de estado del modem GSM
      }
      */
   }


}

/**********************************************************************************/
/*                     Interrupción del Watchdog Timer                            */
/*         Watchdog Timer utilizado como Time out de comunicaciones               */
/**********************************************************************************/
#pragma vector=WDT_VECTOR
__interrupt void intWatchDogTimer(void)
{
  LPM3_EXIT;

  int response=0,m,n;
//  int response;
  char *temp_ch; //, *temp_ch1;

  if(mySystem->ModemGPRS->isModemGPRSOn())
  {
   mySystem->RGBLed->onOnlyBlue();
  }
  response=mySystem->ModemGPRS->timeoutTxRx(mySystem->Reloj,mySystem->getSerialNumber(), mySystem->loggerSAT, mySystem->MPG2, mySystem->Mem);
  switch(response)
  {
  case 1:
/*          if(mySystem->ModemGPRS->stateGPRS&BIT4)  // evalúa si hay solicitud de envío de reporte unavez que se reinició el módem GSM
          {
            mySystem->ModemGPRS->stateGPRS&=~BIT4;
            mySystem->ItzyFlow->requestData=true;
            mySystem->ModemGPRS->OnModemGPRS();
            mySystem->ModemGPRS->connectGPRS();
          }
          else
          {*/
            mySystem->ModemGPRS->turnOFF_VCC_GPRS();
            mySystem->ModemGPRS->UART->statePort&=~0x02;
            mySystem->ModemGPRS->counterTimeout=0;
            mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
            mySystem->MPG2->requestData=false;  // Finaliza transmisiones
            mySystem->ModemGPRS->UART->off_RS_232();
            mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
            mySystem->RGBLed->offAll();
            mySystem->Reloj->startReloj();
//          }
          break;

  case 0: // Espera timeout de comunicaciones o continuación con el siguiente comando
            mySystem->RGBLed->offAll();
  default:  break;
  }
//-----------------
  char qtyMed;
//  mySystem->RGBLed->onOnlyBlue();
  response=mySystem->MPG2->timeoutTxRx(mySystem->getSerialNumber(),mySystem->loggerSAT,mySystem->Reloj,mySystem->Mem,mySystem->ModemGPRS->getDNS(),mySystem->ModemGPRS->getIPPort(),mySystem->ModemGPRS->getAPN(),mySystem->ModemGPRS->getAPNUser(),mySystem->ModemGPRS->getAPNPSW(),mySystem->ModemGPRS->getIDSP());
  switch (response)
  {
//    case 1: // Finaliza transmisiones
//          mySystem->ScadaPk->requestData=false;
//          break;
    case 2:
          mySystem->loggerSAT->setReqLoggerFlg();

            qtyMed=*mySystem->MPG2->getNMedidores();
            mySystem->loggerSAT->clearReqLoggerFlg();

            for(m=0;m<qtyMed;m++)  // m define el número de medidor
            {
              n=0;  // Define posición sobre el buffer
              response=strlen((char*)&mySystem->MPG2->UART->Buffer_Tx[40]);
              if((*mySystem->MPG2->Flowmeter[m]->getPointerWrite()+response)<(*mySystem->MPG2->Flowmeter[m]->getEEPROMstartAddr(m)+0x1000))
              {
                if((*mySystem->MPG2->Flowmeter[m]->getPointerWrite()==*mySystem->MPG2->Flowmeter[m]->getEEPROMstartAddr(m)) && m==0)
                {
                  mySystem->MPG2->UART->Buffer_Rx[n++]=mySystem->MPG2->get_ID_Rep();
                  mySystem->MPG2->UART->Buffer_Rx[n++]=',';
                  strncpy((char*)&mySystem->MPG2->UART->Buffer_Rx[n],mySystem->getSerialNumber(),10);
                  n+=10;
                  mySystem->MPG2->UART->Buffer_Rx[n++]=',';
                  mySystem->MPG2->UART->Buffer_Rx[n++]='0';
                  mySystem->MPG2->UART->Buffer_Rx[n++]='0';
                  mySystem->MPG2->UART->Buffer_Rx[n++]='0';

                  response=*mySystem->MPG2->Flowmeter[m]->getIDVariable();  // 0x01 registro normal, 0x02 registro de alama de puerta abierta

                  sprintf((char*)&mySystem->MPG2->UART->Buffer_Rx[n],",%.1d",response);
                  n+=2;

                  response=*mySystem->Reloj->getTimeZone();

                  sprintf((char*)&mySystem->MPG2->UART->Buffer_Rx[n],",%.2d,",response);
                  n+=4;
                  if(response < 0)
                  {
                    n++;
                  }

                }

                temp_ch=(char*)mySystem->loggerSAT->getCopiedDateTime();
                sprintf((char*)&mySystem->MPG2->UART->Buffer_Rx[n],"%.2d/",*(temp_ch));
                n+=4;
                sprintf((char*)&mySystem->MPG2->UART->Buffer_Rx[n],"%.4d,%.2d:%.2d:%.2d,",*(int*)(temp_ch+3),*(temp_ch+4),*(temp_ch+5),*(temp_ch+6));
                *temp_ch=mySystem->MPG2->UART->Buffer_Rx[n+2];
                sprintf((char*)&mySystem->MPG2->UART->Buffer_Rx[n-1],"%.2d/",*(temp_ch+1));
                mySystem->MPG2->UART->Buffer_Rx[n+2]=*temp_ch;
                n+=14;

                response=*mySystem->MPG2->Flowmeter[m]->getIDVariable();
                sprintf((char*)&mySystem->MPG2->UART->Buffer_Rx[n],"%.1d,",response);
                n+=2;

                strcpy((char*)&mySystem->MPG2->UART->Buffer_Rx[n],(char*)&mySystem->MPG2->UART->Buffer_Tx[40]);

                n=strlen((char*)mySystem->MPG2->UART->Buffer_Rx);

                mySystem->MPG2->UART->Buffer_Rx[n++]=',';
                if(mySystem->door->evalIsDoorOpen()) // Almacena estado de la puerta "O"=open,"C"=close
                {
                  mySystem->MPG2->UART->Buffer_Rx[n++]='O';
                }
                else
                {
                  mySystem->MPG2->UART->Buffer_Rx[n++]='C';
                }

                mySystem->MPG2->UART->Buffer_Rx[n++]=',';

                if(mySystem->VLevelBatt->getVpot()>=Lev_Full)
                {mySystem->MPG2->UART->Buffer_Rx[n++]='F';}
                else
                {
                  if(mySystem->VLevelBatt->getVpot()>=Lev_Low)
                  {mySystem->MPG2->UART->Buffer_Rx[n++]='M';}
                  else
                  {
                    if(mySystem->VLevelBatt->getVpot()>=Lev_Very_Low)
                    {mySystem->MPG2->UART->Buffer_Rx[n++]='L';}
                    else
                    {
                      if(mySystem->VLevelBatt->getVpot()>=Lev_Disch_Comm)
                      {mySystem->MPG2->UART->Buffer_Rx[n++]='D';}
                      else
                      {mySystem->MPG2->UART->Buffer_Rx[n++]='E';}
                    }
                  }
                }

                /*
                #define Lev_Full        0.704  // F: Full
                #define Lev_Low         0.688  // M: Medium
                #define Lev_Very_Low    0.674  // L: Low
                #define Lev_Disch_comm  0.670  // D: Dischcarged
                                               // E: Empty
                */
                //sprintf((char*)&mySystem->MPG2->UART->Buffer_Rx[n],",%.2f\r\n", mySystem->VLevelBatt->getVpot());

                mySystem->MPG2->UART->Buffer_Rx[n++]=0x0D;
                mySystem->MPG2->UART->Buffer_Rx[n++]=0x0A;
                mySystem->MPG2->UART->Buffer_Rx[n++]='\0';

                mySystem->MPG2->UART->Buffer_Tx[40]='\0';
                n=strlen((char*)mySystem->MPG2->UART->Buffer_Rx);

                if((*mySystem->MPG2->Flowmeter[m]->getPointerWrite()+n)<(*mySystem->MPG2->Flowmeter[m]->getEEPROMstartAddr(m)+0x1000))
                {
                  mySystem->loggerSAT->loggingData(*mySystem->MPG2->Flowmeter[m]->getPointerWrite(),(char*)mySystem->MPG2->UART->Buffer_Rx, n, mySystem->Mem); // Almacena datos en memeoria EEPROM I2C sección logger Satelital.
                  mySystem->MPG2->Flowmeter[m]->addPointerWrite(n);
                  mySystem->loggerSAT->incNData();
                }
              }
            }

          mySystem->saveToEEPROM();
          mySystem->MPG2->requestData=false;
          mySystem->ModemGPRS->UART->off_RS_232();
          mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
          mySystem->RGBLed->offAll();
          mySystem->Reloj->startReloj();

          break;
/*  case 3:
          // Código en caso de que el enlace de comunicaciones de telemetría con Scadapack se rompa (No hay comunicaciones)
    if(mySystem->loggerSAT->isReqLogger())
    {
       mySystem->loggerSAT->clearReqLoggerFlg();
          if(*mySystem->ItzyFlow->Flowmeter[0]->getPointerWrite() == 0x060000)
          {
            mySystem->loggerSAT->setNDataBrkLnk();
            mySystem->ItzyFlow->restartWRPointers();

            *(double*)&mySystem->ModemGPRS->UART->Buffer_Rx[n]=*mySystem->loggerSAT->getCopiedDateTime();
                  mySystem->ModemGPRS->UART->Buffer_Rx[1]&=0x0F; // mes
                  mySystem->ModemGPRS->UART->Buffer_Rx[3]&=0x0F; // MSB año

                  response=*mySystem->Reloj->getTimeZone();
                  if(response < 0)
                  {
                    response*=-1;
                    mySystem->ModemGPRS->UART->Buffer_Rx[1]|=0x80; // Establece bandera de time zone negativo en el byte correspondiente al mes
                  }
                  mySystem->ModemGPRS->UART->Buffer_Rx[3]|=((response*0x10) & 0xF0); // Establece el valor del diferencia de horario (timezone) en el byte correspondiente a la parte alta del año


            mySystem->ModemGPRS->UART->Buffer_Rx[7]=((ID_Producto & 0x3F) | 0x40);  // 0x40 es ID para detección de alarma de desconexión de dispositivo ScadaPack
            n=8;
            mySystem->loggerSAT->loggingData(*mySystem->ItzyFlow->Flowmeter[0]->getPointerWrite(),(char*)mySystem->ModemGPRS->UART->Buffer_Rx, n, mySystem->Mem); // Almacena datos en memeoria EEPROM I2C sección logger Satelital en sección asignada al dispositivo 1 (Medidor 1)

            mySystem->saveToEEPROM();
          }
    }
    */
/*  case 4://Cambio de DNS a conectarse y puerto IP
          temp_ch=strstr((char*)mySystem->MPG2->UART->Buffer_Rx,":");
          if(temp_ch)
          {
            temp_ch++;
            temp_ch1=strstr(temp_ch,",");
            m=temp_ch1-temp_ch;
            strncpy(mySystem->ModemGPRS->DNS,temp_ch,m);
            mySystem->ModemGPRS->DNS[m]='\0';

            temp_ch1++;
            //temp_ch=strstr(temp_ch1,"\0");
            //m=temp_ch-temp_ch1;
            strcpy(mySystem->ModemGPRS->IP_PORT,temp_ch1);
            //mySystem->ModemGPRS->IP_PORT[m]='\0';
            mySystem->MPG2->setFlagTXExt();
            mySystem->MPG2->UART->startTx("OK\r\n",4);
          }
          mySystem->saveToEEPROM();
//          mySystem->ModemGPRS->counterTimeout=0;
//          mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
          mySystem->MPG2->requestData=false;
          mySystem->MPG2->reInitComms();
          mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
          mySystem->Reloj->startReloj();
          break;
*/
/*  case 3:// Cambio de APN, USER & PSW
          temp_ch=strstr((char*)mySystem->MPG2->UART->Buffer_Rx,":");
          if(temp_ch)
          {
            temp_ch++;
            temp_ch1=strstr(temp_ch,",");
            m=temp_ch1-temp_ch;
            strncpy(mySystem->ModemGPRS->APN_GPRS_SERV,temp_ch,m);
            mySystem->ModemGPRS->APN_GPRS_SERV[m]='\0';

            temp_ch1++;
            temp_ch=strstr(temp_ch1,",");
            m=temp_ch-temp_ch1;
            strncpy(mySystem->ModemGPRS->APN_GPRS_USER,temp_ch1,m);
            mySystem->ModemGPRS->APN_GPRS_USER[m]='\0';

            temp_ch++;
//            temp_ch1=strstr(temp_ch,"\0");
//            m=temp_ch1-temp_ch;
            strcpy(mySystem->ModemGPRS->APN_GPRS_PSW,temp_ch);
//            mySystem->ModemGPRS->APN_GPRS_PSW[m]='\0';
            mySystem->ModemGPRS->ID_SERV_PROV=0x03;
            mySystem->MPG2->setFlagTXExt();
            mySystem->MPG2->UART->startTx("OK\r\n",4);
          }
          mySystem->saveToEEPROM();
//          mySystem->ModemGPRS->counterTimeout=0;
//          mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
          mySystem->MPG2->requestData=false;
          mySystem->MPG2->reInitComms();
          mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
          mySystem->Reloj->startReloj();
          break;
    */
    case 1:
//          mySystem->ModemGPRS->counterTimeout=0;
//          mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
          mySystem->MPG2->requestData=false;
          mySystem->ModemGPRS->UART->off_RS_232();
          mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
          mySystem->RGBLed->offAll();
          mySystem->Reloj->startReloj();
          //mySystem->Reloj->startReloj();
          //break;
    case 0: //Espera de timeout de comunicaciones o continuación del siguiente comando (no hay espera de falta de datos)
          mySystem->RGBLed->offAll();
    default: break;
  }
}
/**********************************************************************************/
/**********************************************************************************/
/*                       Comunicaciones UART0 por hardware                        */
/*             Rutina de interrupción Tx para módulo Satelital Q1000              */
/**********************************************************************************/
#pragma vector=USART0TX_VECTOR
__interrupt void intUART0Tx(void)
{
  LPM3_EXIT;
  if(mySystem->ModemGPRS->UART->statePort&0x02)
  {
    mySystem->ModemGPRS->UART->dataPtrTx++;
    if ((mySystem->ModemGPRS->UART->dataPtrTx) < mySystem->ModemGPRS->UART->strTxLen)
    {
       U0TXBUF=mySystem->ModemGPRS->UART->Buffer_Tx_Start[mySystem->ModemGPRS->UART->dataPtrTx];
    }
    else
    {
      if ((mySystem->ModemGPRS->UART->dataPtrTx) == mySystem->ModemGPRS->UART->strTxLen)
      { while(!(U0TCTL&TXEPT))
        {
        }
        mySystem->ModemGPRS->UART->myWDT->restartWdt();
      }
    }
  }
}
/**********************************************************************************/
/*                       Comunicaciones UART0 por hardware                        */
/*             Rutina de interrupción Rx para módulo Satelital Q1000              */
/**********************************************************************************/
#pragma vector=USART0RX_VECTOR
__interrupt void intUART0Rx(void)
{
  LPM3_EXIT;
 // if(mySystem->ModemGPRS->isModemGPRSOn())
 // {
    TACTL&=~TAIE; // Deshabilita la interrupcion para incremento de reloj.
 // }
  if (mySystem->ModemGPRS->UART->dataPtrRx==buffer0_size || mySystem->ModemGPRS->UART->UartError!=0x00)
  {
     mySystem->ModemGPRS->UART->dataPtrRx=0;      // Inicializa el puntero del buffer de Rx si hay un overflow
     mySystem->ModemGPRS->UART->UartError|=0x01;  // Activa bandera de error de overflow.
     TACTL|=TAIE;
  }
  else
  {
     mySystem->ModemGPRS->UART->statePort|=0x02;
     //mySystem->ModemGPRS->UART->on_RS_232();
     mySystem->ModemGPRS->UART->Buffer_Rx[mySystem->ModemGPRS->UART->dataPtrRx]=U0RXBUF;
     mySystem->ModemGPRS->UART->dataPtrRx++;
     mySystem->ModemGPRS->UART->strRxLen=mySystem->ModemGPRS->UART->dataPtrRx;
  }

  mySystem->ModemGPRS->UART->myWDT->restartWdt();
}
/**********************************************************************************/
/**********************************************************************************/
/*                       Comunicaciones UART1 por hardware                        */
/*                     Rutina de interrupción Tx para Medidor                     */
/**********************************************************************************/
#pragma vector=USART1TX_VECTOR
__interrupt void intUART1Tx(void)
{
  LPM3_EXIT;
  if(mySystem->MPG2->UART->statePort&0x02)
  {
    mySystem->MPG2->UART->dataPtrTx++;
    if ((mySystem->MPG2->UART->dataPtrTx) < mySystem->MPG2->UART->strTxLen)
    {
       U1TXBUF=mySystem->MPG2->UART->Buffer_Tx_Start[mySystem->MPG2->UART->dataPtrTx];
    }
    else
    {
      if ((mySystem->MPG2->UART->dataPtrTx) == mySystem->MPG2->UART->strTxLen)
      { while(!(U1TCTL&TXEPT))
        {
        }
        //mySystem->Medidor->UART->statePort&=~0x02;
        mySystem->MPG2->UART->myWDT->restartWdt();
      }
    }
  }

}
/**********************************************************************************/
/*                       Comunicaciones UART1 por hardware                        */
/*                     Rutina de interrupción Rx para Medidor                     */
/**********************************************************************************/
#pragma vector=USART1RX_VECTOR
__interrupt void intUART1Rx(void)
{
  LPM3_EXIT;

  if (mySystem->MPG2->UART->dataPtrRx==buffer1_size || mySystem->MPG2->UART->UartError!=0x00)
  {
     mySystem->MPG2->UART->dataPtrRx=0;      // Inicializa el puntero del buffer de Rx si hay un overflow
     mySystem->MPG2->UART->UartError|=0x01;  // Activa bandera de error de overflow.
  }
  else
  {
     mySystem->MPG2->UART->statePort|=0x02;
     //mySystem->ModemGPRS->UART->on_RS_232();
     mySystem->MPG2->UART->Buffer_Rx[mySystem->MPG2->UART->dataPtrRx]=U1RXBUF;
     if(!mySystem->MPG2->UART->dataPtrRx)
     {
       mySystem->Reloj->stopReloj();
     }
     mySystem->MPG2->UART->dataPtrRx++;
     mySystem->MPG2->UART->strRxLen=mySystem->MPG2->UART->dataPtrRx;
  }

  mySystem->MPG2->UART->myWDT->restartWdt();
}
/**********************************************************************************/
/*                            Interrupción Timer B                                */
/*                     Rutina de generador de tiempo xxxxxxxx                     */
/**********************************************************************************/
#pragma vector=TIMERB0_VECTOR
__interrupt void intTimerB0(void)
{

  LPM3_EXIT;

  //int k;

  P5OUT^=0x01;
  TBCCTL0&=~CCIFG;

  /*if(!mySystem->m_btnProg->isProgFlagActivated())
  {*/
  mySystem->ModemGPRS->counterTimeout++;

/*  if(mySystem->ModemGPRS->isModemGPRSOn())
  {
   mySystem->RGBLed->onOnlyBlue();
  }*/

  switch(mySystem->ModemGPRS->counterTimeout)
  {
  case 1800:
            mySystem->RGBLed->offAll();
            if(mySystem->ModemGPRS->isModemGPRSOn()) // el modem se encuentra prendido?
            {
              mySystem->ModemGPRS->OFFfromGPRSSession();
            }
            else
            {
              mySystem->ModemGPRS->counterTimeout=0;
              mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
              mySystem->ModemGPRS->UART->statePort&=~0x02;
              mySystem->ModemGPRS->stateGPRS&=~(BIT0|BIT1|BIT3|BIT5); // Deshabilita bandera de solicitud de comando AT
              mySystem->ModemGPRS->UART->UartError&=~0x01;
              mySystem->ModemGPRS->UART->clearRxBuffPointer(mySystem->ModemGPRS->UART); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
              mySystem->ModemGPRS->UART->strRxLen=0;
              mySystem->ModemGPRS->UART->clearBufferRx();
              mySystem->ModemGPRS->UART->extraTimeCounter=0;
              mySystem->ModemGPRS->UART->commTries=0;
              mySystem->MPG2->requestData=false;
              mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
              mySystem->Reloj->startReloj();
            }
            break;
  case 3600:
            mySystem->RGBLed->offAll();
            if(mySystem->ModemGPRS->isModemGPRSOn()) // el modem se encuentra prendido?
            {
              mySystem->ModemGPRS->counterTimeout=0;
              mySystem->ModemGPRS->OffModemGPRS();
            }
            else
            {
              mySystem->ModemGPRS->counterTimeout=0;
              mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
              mySystem->ModemGPRS->UART->statePort&=~0x02;
              mySystem->ModemGPRS->stateGPRS&=~(BIT0|BIT1|BIT3|BIT5); // Deshabilita bandera de solicitud de comando AT
              mySystem->ModemGPRS->UART->UartError&=~0x01;
              mySystem->ModemGPRS->UART->clearRxBuffPointer(mySystem->ModemGPRS->UART); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
              mySystem->ModemGPRS->UART->strRxLen=0;
              mySystem->ModemGPRS->UART->clearBufferRx();
              mySystem->ModemGPRS->UART->extraTimeCounter=0;
              mySystem->ModemGPRS->UART->commTries=0;
              mySystem->MPG2->requestData=false;
              mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
              mySystem->Reloj->startReloj();
            }
            break;
  default:  break;
  }
 /* }
  else
  {
      if(mySystem->m_btnProg->isCounterAtLimit()) // Cumplió 2 minutos?
      {// Desabilita el dispsitivo RS-232 por completo
        mySystem->ModemGPRS->UART->off_RS_232();
        mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
        mySystem->m_btnProg->clearProgFlag();
      }
      else
      {
        mySystem->ModemGPRS->UART->on_RS_232();
      }
  }*/
}
/**********************************************************************************/
/*                            Interrupción Timer A                                */
/*             Rutina de generador de tiempo de 1 seg externo RTC                 */
/**********************************************************************************/
#pragma vector=TIMERA1_VECTOR
__interrupt void intTimerA1(void)
{
//  unsigned long dataSize;
  LPM3_EXIT;
  //P5OUT^=0x01;
  TACTL&=~TAIFG;

  mySystem->RGBLed->changeStateOnLED();

  if(!mySystem->m_btnProg->isProgFlagActivated())
  {
    if(mySystem->RGBLed->isTimeToLighting())
    {
      if(mySystem->VLevelBatt->getVpot()>=Lev_Disch_Comm)
      {
        mySystem->RGBLed->onOnlyGreen();
      }
      else
      {
        mySystem->RGBLed->onOnlyRed();
      }
    }
  }
  else
  {
    mySystem->RGBLed->onWhite();
  }

   if((mySystem->ModemGPRS->stateGPRS&BIT4) && !mySystem->ModemGPRS->isModemGPRSOn() && !mySystem->m_btnProg->isProgFlagActivated())  // evalúa si hay solicitud de envío de reporte una vez que se reinició el módem GSM
   {
      mySystem->ModemGPRS->stateGPRS&=~BIT4;
      if(*mySystem->loggerSAT->getPNDataBCD())
      {
        mySystem->Reloj->stopReloj();
        mySystem->MPG2->requestData=true;
        mySystem->ModemGPRS->counterTimeout=0;
        mySystem->ModemGPRS->timerReset->EnableTBCCTL0_IE();
        mySystem->ModemGPRS->OnModemGPRS();
        mySystem->ModemGPRS->connectGPRS();
      }
      else
      {
        mySystem->MPG2->requestData=false;
      }
   }

  // Lineas de incremento de reloj interno
  if(!mySystem->menuSelection)
  {
    if(!mySystem->Mem->secCounter)
    {mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);}
    mySystem->Reloj->incClock(1);
    //mySystem->Reloj->incClockWLCD(mySystem->LCD, 1);
    //mySystem->LCD->setAddr(0x93);
    //mySystem->LCD->setCaracter(0x30+*mySystem->MPG2->getNMedidores());

    //mySystem->LCD->setAddr(0xC3);
    //mySystem->LCD->setCaracter(0x30+((mySystem->loggerSAT->getNDataBCD()&0xF0)/0x10));
    //mySystem->LCD->setAddr(0xC4);
    //mySystem->LCD->setCaracter(0x30+(mySystem->loggerSAT->getNDataBCD()&0x0F));

    //mySystem->LCD->setAddr(0xCF);
    /*if(mySystem->ModemGPRS->isEnableSendingReport())
    {
      mySystem->LCD->setCaracter(0x20);
    }
    else
    {
      mySystem->LCD->setCaracter('*');
    }*/

  }
  else
  {
     mySystem->Reloj->incClock(1);
  }

  // Fin de lineas para reloj interno

  // Lineas para data logger interno y para reporte satelital

/*  if(mySystem->loggerSAT->isEnabledLogg())
  {
    mySystem->loggerSAT->incCounterSmplTime();
  }
*/

  if((mySystem->MPG2->requestData==false && mySystem->loggerSAT->isTimeSampleOn(mySystem->Reloj) && mySystem->loggerSAT->isEnabledLogg() && !mySystem->m_btnProg->isProgFlagActivated()))
  {
    mySystem->loggerSAT->copyDateTime(mySystem->Reloj);
    mySystem->MPG2->requestData=true;
    mySystem->MPG2->set_ID_Rep(0x32);
    mySystem->MPG2->Flowmeter[0]->setIDVariable(0x01);

    if(mySystem->loggerSAT->getNDataBCD()==0xFF)
    {
      mySystem->loggerSAT->clearNData();
    }
    mySystem->Reloj->stopReloj();
    mySystem->MPG2->clearCommCounter();
    mySystem->ModemGPRS->UART->myWDT->restartWdt();
    mySystem->MPG2->sendFrameToMPG2(mySystem->MPG2->getCommCounter());
  }

  // Fin de lineas para data logger de logger interno y para reporte satelital

  // Lineas para detección de cumplimiento de horario de repote vía GPRS
  if(mySystem->MPG2->requestData==false && mySystem->VLevelBatt->getVpot()>=Lev_Disch_Comm && !mySystem->m_btnProg->isProgFlagActivated()) // Existe permiso para solicitar comunicaciones y el voltaje de la batería se encuentra en condicones para comunicar?
  {
    if(mySystem->Reloj->compareRegAlarm()) // Genera Alarma?
    {
      /*if(mySystem->ModemGPRS->isEnableSendingReport())
      {*/
        mySystem->MPG2->requestData=true;
        mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
        mySystem->Reloj->stopReloj();
        mySystem->ModemGPRS->counterTimeout=0;
        mySystem->ModemGPRS->timerReset->EnableTBCCTL0_IE();

        if(mySystem->ModemGPRS->isIPSessionOpen() && mySystem->ModemGPRS->isModemGPRSOn())
        {
           mySystem->ModemGPRS->stateGPRS|=BIT4;  // Habilita bandera de apagar el sistema

           if(mySystem->ModemGPRS->isIPSessionOpen() && mySystem->ModemGPRS->stateGPRS&BIT5)
           {
                mySystem->ModemGPRS->OFFfromGPRSSession();
           }
           else
           {// Se ejecuto normalmemte al iniciar por primera vez el sistema o restetearlo (El modem está encendido)
             mySystem->ModemGPRS->OffModemGPRS();
           }
        }
        else
        {
          if(mySystem->ModemGPRS->isModemGPRSOn())
          {
            mySystem->ModemGPRS->stateGPRS|=BIT4;  // Habilita bandera de apagar el sistema
            mySystem->ModemGPRS->OffModemGPRS();
          }
          else
          {
            if(*mySystem->loggerSAT->getPNDataBCD())
            {
              mySystem->ModemGPRS->OnModemGPRS();
              mySystem->ModemGPRS->connectGPRS();
            }
            else  // No realiza comunicaciones con modem GSM
            {
              mySystem->ModemGPRS->counterTimeout=0;
              mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
              mySystem->MPG2->requestData=false;  // Finaliza transmisiones
              mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
              mySystem->Reloj->startReloj();
            }
          }
        }
/*      }
      else
      {
        //mySystem->loggerSAT->clearCounterSmplTime();
        mySystem->ItzyFlow->restartWRPointers();
        mySystem->loggerSAT->clearNData();
        mySystem->saveToEEPROM();
      }*/
    }

  }
  // Fin de lineas para detección de cumplimiento de horario de reporte vía Satelital

  // Lineas para respado de sistema en un periodo de tiempo
  mySystem->Mem->secCounter++;
  if(mySystem->Mem->secCounter>=3600) // 3600 segundos (1 hr) para respaldar información importante para recuperar al inicio del sistema
  {
    mySystem->Mem->secCounter=0;
    //mySystem->saveToEEPROM();
  }
  // Fin de lineas para resplado de sistema en un periodo de tiempo

  // Lineas de periodo de tiempo de encendido del backlight del LCD
  /*mySystem->LCD->incBLCounter();
  if(mySystem->LCD->getBLCounter()>=10)
  {
    mySystem->LCD->clearBLCounter();
    mySystem->LCD->offBackLight();
  }*/
  // Fin de lineas de periodo de tiempo de encendido del backlight del LCD

  // Inicio de lineas de evaluación de estado de la puerta de la telemetría
  if(mySystem->MPG2->requestData==false && !mySystem->m_btnProg->isProgFlagActivated())
  {
    if(mySystem->door->evalIsDoorOpen())
    {// Puerta Abierta
      mySystem->door->incCounterTime();
      if(mySystem->door->isCounterFull())
      { // Generar mensaje de alerta de puerta y almacenarlo en el logger de registros
        if(!mySystem->door->getDoorStateFlg())
        {
          mySystem->door->setDoorOpenStateFlg();
          mySystem->MPG2->set_ID_Rep(0x32);
           mySystem->MPG2->Flowmeter[0]->setIDVariable(0x02);
          mySystem->loggerSAT->copyDateTime(mySystem->Reloj);
          mySystem->MPG2->requestData=true;

          if(mySystem->loggerSAT->getNDataBCD()==0xFF)
          {
            mySystem->loggerSAT->clearNData();
          }
          mySystem->Reloj->stopReloj();
          mySystem->MPG2->clearCommCounter();
          mySystem->ModemGPRS->UART->myWDT->restartWdt();
          mySystem->MPG2->sendFrameToMPG2(mySystem->MPG2->getCommCounter()); // New Line
        }
      }
    }
    else
    {// Puerta Cerrada
      mySystem->door->clearCounterTime();
      if(mySystem->door->getDoorStateFlg())
      {
        mySystem->door->setDoorCloseStateFlg();
      }
    }
  }

  // Fin de lineas de evaluación de estaado de la puerta de la telemetría

  // Inicio de lineas de evaluación de tiempo para obtner el voltaje de la batería
  if(mySystem->MPG2->requestData==false)
  {
    mySystem->VLevelBatt->incAndCompCounter();
  }

  mySystem->RGBLed->offAll();

}
/**********************************************************************************/
/*                            Interrupción Timer A                                */
/*                     Rutina de generador de tiempo de 0.25 seg                  */
/**********************************************************************************/
/*#pragma vector=TIMERA1_VECTOR
__interrupt void intTimerA(void)
{
  LPM3_EXIT;
  if(!(mySystem->KBoard->KBFlags&0x01))
  {
    mySystem->KBoard->disableTempA();
    TACTL&=~TAIFG;
    mySystem->KBoard->enableKBoard();
  }
}*/
/**********************************************************************************/
/*                           Interrupción Puerto 1                                */
/*                            Rutina para teclado                                 */
/**********************************************************************************/
#pragma vector=PORT1_VECTOR
__interrupt void PORT1ISR (void)
{
 LPM3_EXIT;

 for(int i=0;i<2000;i++)
 {_NOP();}

 if(mySystem->m_btnProg->isButtonPressed() && mySystem->MPG2->requestData==false)
 {
   mySystem->MPG2->setEncryptionMode();
   mySystem->ModemGPRS->UART->on_RS_232();
   //mySystem->ModemGPRS->timerReset->EnableTBCCTL0_IE();
   mySystem->m_btnProg->setProgFlag();
 }
/****************************/
/*    mySystem->loggerSAT->copyDateTime(mySystem->Reloj);
    mySystem->MPG2->requestData=true;
    mySystem->MPG2->set_ID_Rep(0x32);
    mySystem->MPG2->Flowmeter[0]->setIDVariable(0x01);

    if(mySystem->loggerSAT->getNDataBCD()==0xFF)
    {
      mySystem->loggerSAT->clearNData();
    }
    mySystem->Reloj->stopReloj();
    mySystem->MPG2->clearCommCounter();
    mySystem->ModemGPRS->UART->myWDT->restartWdt();
    mySystem->MPG2->sendFrameToMPG2(mySystem->MPG2->getCommCounter());
 */
/****************************/
 P1IFG=0x00;
}
/**********************************************************************************/
/*                              Interrupción DMA                                  */
/*                            Rutina para TX UARTS                               */
/**********************************************************************************/
#pragma vector=DACDMA_VECTOR
__interrupt void UARTS_O_1(void)
{
  LPM3_EXIT;

  if(DMA0CTL&DMAIFG)
  {
    mySystem->ModemGPRS->UART->myWDT->restartWdt();
  }

  if(!mySystem->MPG2->getFlagTXExt())
  {
    if(DMA1CTL&DMAIFG)
    {
      mySystem->ModemGPRS->UART->myWDT->restartWdt();
    }
  }
  else
  {
    /*if(mySystem->m_btnProg->isProgFlagActivated())
    {
      mySystem->ModemGPRS->UART->on_RS_232();
      mySystem->ModemGPRS->timerReset->EnableTBCCTL0_IE();
      mySystem->m_btnProg->setProgFlag();
      mySystem->m_btnProg->clearCounterSeconds();
    }*/
    mySystem->MPG2->clearFlagTXExt();
    mySystem->saveToEEPROM();
    mySystem->MPG2->requestData=false;
    mySystem->MPG2->reInitComms();
    mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
    mySystem->Reloj->startReloj();
  }
  DMA0CTL&=~DMAIFG;
  DMA1CTL&=~DMAIFG;
/*  DMA0CTL&=~DMAIFG;
  DMA1CTL&=~DMAIFG;
  mySystem->ModemGPRS->UART->myWDT->restartWdt();
  */
}
/**********************************************************************************/
#pragma vector = ADC12_VECTOR
__interrupt void intADC12(void)
{
  LPM3_EXIT;

  mySystem->VLevelBatt->getSample();
  if(TACTL&TAIFG)
  {
    TACTL&=~TAIFG;
  }
}
/**********************************************************************************/
