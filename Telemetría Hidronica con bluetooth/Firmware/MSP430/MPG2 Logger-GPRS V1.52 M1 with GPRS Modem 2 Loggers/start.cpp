/**********************************************************************************************/
/* Hidronica S.A. de C.V.                                                                     */
/* Programador: Ing. Fernando Bolaños G.                                                      */
/* Fecha de desarrollo: Agosto 2017                                                           */
/* Programa: MPG2 Logger-GPR Norma CNA                                                        */
/* Version: 1.0                                                                               */
/* Comunica con modem Quectel UC20 o EC20 y medidor MPG2                                      */
/* Almacena registros de gasto y volumen en memoria SD                                        */
/**********************************************************************************************/
/* Stack Size: 500 Bytes                                                                      */
/* Heap size: 8000 Bytes                                                                      */
/* Double Floating-point 64 bits                                                              */
/**********************************************************************************************/

/**********************************************************************************************/
/*                                 Inicio de Programa                                         */
/**********************************************************************************************/
#include "CModule.h"

__no_init CModule *mySystem @ 0x1100;

//const char* commandStr[] ={"HWCD;2701150002;www.gprshost.tzo.com;2007;1C"};

int main(void)
{
  unsigned long int k, l_NCluster;
  int x;

  mySystem=new CModule();

  // mySystem->ModemGPRS->calcCRC((char*)*commandStr,42);

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

/**************************************/
    mySystem->MPG2->clearCommCounter();
    mySystem->MPG2->UART->myWDT->restartWdt();

    mySystem->m_Fat->SDMem->startInitializeMem();
    mySystem->m_Fat->readBootBlock(); // Reads partition sector
    mySystem->m_Fat->readSystemPartition(); // Reads LBA where partiton begins in memory (Reads FAT first sector)

/**************************************/
/*   mySystem->ModemGPRS->counterTimeout=0;
   mySystem->ModemGPRS->timerReset->EnableTBCCTL0_IE();
   mySystem->ModemGPRS->OnModemGPRS();
   mySystem->MPG2->set_ID_Rep(0x31);
   mySystem->MPG2->requestData=true;
   mySystem->ModemGPRS->setupGPRS();
*/
/**************************************/
   mySystem->Reloj->startReloj();

   while(true)
   {
      LPM3;             // Low Power Mode 3

      //Sección para el envío de información por los puertos serie cuando existe una activación de horario de reporte o Alarma
      switch(mySystem->flagsAlarms)
      {
      case BIT0: // Alarma para recuperar infromación del medidor
                //mySystem->flagsAlarms|=BIT0;
                mySystem->flagsAlarms=0x00;
                mySystem->MPG2->sendFrameToMPG2();
                break;
      case BIT1:
                mySystem->flagsAlarms=0x00;
                mySystem->ModemGPRS->OnModemGPRS();
                mySystem->ModemGPRS->connectGPRS();
                break;
      case BIT2:
                mySystem->flagsAlarms=0x00;
                mySystem->ModemGPRS->OffModemGPRS();
                break;
      case BIT3:
                mySystem->flagsAlarms=0x00;
                mySystem->ModemGPRS->OFFfromGPRSSession();
                break;
      default:  mySystem->flagsAlarms=0x00;
                break;
      }
      //////////////////////////////////////////////////////////////////
      if(*mySystem->ModemGPRS->getFlags_reqInfo() & BIT0)
      { // Rutina para leer de la memoria SD y enviarla via Bluetooth
        mySystem->ModemGPRS->flags_req_info&=~BIT0;
        mySystem->Reloj->stopReloj();
        mySystem->m_Fat->SDMem->startInitializeMem();
        mySystem->m_Fat->readFAT(mySystem->m_Fat->SDMem->RD_Addr);


        for(x=0;x<50;x++)
        {
          if(mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[x]==0xFE)
          {
            break;
          }
        }
        if(x<50)
        {
          x++;
          //mySystem->Reloj->stopReloj();
          mySystem->ModemGPRS->UART->enableTXInterrupt();
          mySystem->ModemGPRS->sendStringToUART((char*)&mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[x], 512);
        }
        else
        {
          mySystem->ModemGPRS->UART->disableTXInterrupt();
          mySystem->Reloj->startReloj();
        }
      }
      /*********************************************************************************/
      if(*mySystem->ModemGPRS->getFlags_reqInfo() & BIT1)
      { // Rutina para borrar un folder de recolección de datos por año en memoria SD
        mySystem->ModemGPRS->flags_req_info&=~BIT1;
        mySystem->Reloj->stopReloj();

        mySystem->m_Fat->SDMem->startInitializeMem();
        mySystem->m_Fat->searchAndDeleteFolder((char*)&mySystem->ModemGPRS->UART->Buffer_Rx[16], mySystem->loggerSAT, mySystem->Mem);

        mySystem->ModemGPRS->UART->Buffer_Rx[4]=':';
        sprintf((char*)&mySystem->ModemGPRS->UART->Buffer_Rx[5],"%s","OK;");
        int respLength=8;
        char m_CRC=0;
        m_CRC=mySystem->ModemGPRS->calcCRC((char*)mySystem->ModemGPRS->UART->Buffer_Rx,respLength);
        sprintf((char *)&mySystem->ModemGPRS->UART->Buffer_Rx[respLength], "%.2X%c%c",m_CRC,'\r','\n');
        respLength+=4;

        for(m_CRC=0;m_CRC<respLength;m_CRC++)
        {
          mySystem->ModemGPRS->UART->Buffer_Tx[m_CRC]=mySystem->ModemGPRS->UART->Buffer_Rx[m_CRC];
        }
        mySystem->ModemGPRS->UART->strRxLen=0;
        mySystem->ModemGPRS->UART->clearBufferRx();

        mySystem->ModemGPRS->sendStringToUART((char*)mySystem->ModemGPRS->UART->Buffer_Tx, respLength);

        mySystem->Reloj->startReloj();
      }

/////////////////////////////
   rewrittingReg:
      if(mySystem->Mem->getNewRegMPFlag())
      {
        mySystem->Reloj->stopReloj();
        // Reads the new register stored on I2C EEPROM
        k=(*(unsigned long int*)mySystem->MPG2->Flowmeter[0]->getEEPROMstartAddr(0));
        for(x=0;x<4;x++)
        {
         mySystem->loggerSAT->readLoggedData((k+(x*128)), (char*)mySystem->MPG2->UART->Buffer_Rx+(x*128), mySystem->Mem);
        }
        // Search for cluster of file according to month and year
        //mySystem->m_Fat->SDMem->startInitializeMem();
        if(mySystem->m_Fat->SDMem->startInitializeMem())
        {
          mySystem->m_Fat->readBootBlock(); // Reads partition sector
          mySystem->m_Fat->readSystemPartition(); // Reads LBA where partiton begins in memory (Reads FAT first sector)
          l_NCluster=mySystem->m_Fat->searchClusterOfFile((char*)&mySystem->MPG2->UART->Buffer_Rx[3]);
          if(l_NCluster)
          {// The file exists
          mySystem->m_Fat->setRegisterOnSDMem((char*)&mySystem->MPG2->UART->Buffer_Rx[3],l_NCluster,mySystem->MPG2, mySystem->loggerSAT, mySystem->Mem);
          mySystem->Mem->clearNewRegMPFlag();
          }
          else
          {// File does´n exist and needs to be created
            //mySystem->m_Fat->SDMem->startInitializeMem();
            mySystem->m_Fat->createsNewYearFolder(mySystem->m_Fat->findSpaceOnMemFolder(0x00000000),(char*)&mySystem->MPG2->UART->Buffer_Rx[6], mySystem->loggerSAT, mySystem->Mem);
            goto rewrittingReg;
          }
        }
        else
        {
          mySystem->Mem->clearNewRegMPFlag();
        }
        mySystem->Reloj->startReloj();
      }
/////////////////////////////

      if(mySystem->VLevelBatt->isFlagReq())
      {
        mySystem->VLevelBatt->clearFlagReq();
        mySystem->VLevelBatt->getVoltageBatt();
      }

      if(mySystem->m_btnProg->isProgFlagActivated())
      {
        if(!mySystem->m_btnProg->isButtonPressed()) // Se deconetó la conexión inalámbrica Bluetooth?
        {// Desabilita el dispsitivo RS-232 por completo
          mySystem->ModemGPRS->clearValidatedPasswordFlag();
          mySystem->m_btnProg->clearProgFlag();
          mySystem->restart_TX_GPRS_BT();
        }
      }

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

  int response=0,m,n,j;
//  unsigned long int memAdd;
  char *temp_ch;

  if(mySystem->m_btnProg->isProgFlagActivated())
  {
     mySystem->ModemGPRS->flags_req_info&=~BIT0;
     mySystem->ModemGPRS->timeOUTExtRxBluetooth(mySystem->Reloj,mySystem->getSerialNumber(), mySystem->loggerSAT, mySystem->loggerGPRS, mySystem->MPG2, mySystem->Mem,mySystem->getLocLatitude(),mySystem->getRFC(),mySystem->getExternalSerialNoFlowMet(),mySystem->getIDFMorIS(),mySystem->getIDUV(),mySystem->getSysPassword(),mySystem->m_Fat);
     if(!(*mySystem->ModemGPRS->getFlags_reqInfo() & BIT0))
     {  // Deshabilita la atención de interrupción de Tx para Bluetooth (deshabilita para enviar cadenas de caracteres largos y manipular la señal RTS)
        mySystem->ModemGPRS->UART->disableTXInterrupt();
        mySystem->Reloj->startReloj();
     }
     if((*mySystem->ModemGPRS->getFlags_reqInfo() & BIT7)) // Almacenar información en memeoria de respaldo EEPROM?
     {
        mySystem->ModemGPRS->flags_req_info&=~BIT7;
        mySystem->saveToEEPROM();
     }
     /*mySystem->ModemGPRS->turnOFF_VCC_GPRS();
     mySystem->ModemGPRS->UART->statePort&=~0x02;
     mySystem->ModemGPRS->counterTimeout=0;
     mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
     mySystem->MPG2->requestData=false;  // Finaliza transmisiones
     mySystem->ModemGPRS->UART->off_RS_232();
     mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
     mySystem->RGBLed->offAll();
     mySystem->saveToEEPROM();
     mySystem->Reloj->startReloj();*/
  }
  else
  {
    if(mySystem->ModemGPRS->isModemGPRSOn())
    {
    mySystem->RGBLed->onOnlyBlue();
    }
    response=mySystem->ModemGPRS->timeoutTxRx(mySystem->Reloj,mySystem->getSerialNumber(), mySystem->loggerGPRS, mySystem->MPG2, mySystem->Mem);
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
              mySystem->restart_TX_GPRS_BT();

              /*mySystem->ModemGPRS->UART->statePort&=~0x02;
              mySystem->ModemGPRS->counterTimeout=0;
              mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
              mySystem->MPG2->requestData=false;  // Finaliza transmisiones
              mySystem->ModemGPRS->UART->off_RS_232();
              mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
              mySystem->RGBLed->offAll();
              mySystem->Reloj->startReloj();*/
//          }
            break;

    case 0: // Espera timeout de comunicaciones o continuación con el siguiente comando
              mySystem->RGBLed->offAll();
    default:  break;
    }
  }
//-----------------
  char qtyMed;
//  mySystem->RGBLed->onOnlyBlue();
  if(mySystem->m_Fat->SDMem->isSPIEnabled())
  {
    //mySystem->Reloj->startReloj();
  }
  else
  {
    //response=mySystem->MPG2->timeoutTxRx(mySystem->getSerialNumber(),mySystem->loggerSAT,mySystem->Reloj,mySystem->Mem,mySystem->ModemGPRS->getDNS(),mySystem->ModemGPRS->getIPPort(),mySystem->ModemGPRS->getAPN(),mySystem->ModemGPRS->getAPNUser(),mySystem->ModemGPRS->getAPNPSW(),mySystem->ModemGPRS->getIDSP());
    response=mySystem->MPG2->timeoutTxRx(mySystem->getSerialNumber(),mySystem->loggerSAT,mySystem->Reloj,mySystem->Mem);
  }
  switch (response)
  {

    case 3: // Finaliza comunicaciones en caso que el medidor no recibió información o la recibió de manera errónea o incompleta
           //mySystem->MPG2->set_ID_Rep(0x34);
           mySystem->MPG2->Flowmeter[0]->setIDVariable(0x04);
           mySystem->MPG2->Flowmeter[1]->setIDVariable(0x04);
           mySystem->MPG2->setFailRegisters((char*)mySystem->MPG2->UART->Buffer_Tx);
      // Generar código para establecer el registro de error en caso que el medidor no haya contestado en el requerimiento de información.
    case 2:
          //mySystem->loggerSAT->setReqLoggerFlg();

            qtyMed=*mySystem->MPG2->getNMedidores();
            //mySystem->loggerSAT->clearReqLoggerFlg();

            for(m=0;m<qtyMed;m++)  // m define el número de medidor
            //if(qtyMed)
            {
              n=0;  // Define posición sobre el buffer
              switch(m)
              {
              case 0:

                //response=strlen((char*)&mySystem->MPG2->UART->Buffer_Tx[40]);
                if(mySystem->loggerSAT->isFlagStoreLogger())
                {
                  mySystem->loggerSAT->clearFlagStoreLogger();
                temp_ch=(char*)mySystem->loggerSAT->getCopiedDateTime();
                sprintf((char*)&mySystem->MPG2->UART->Buffer_Rx[n],"%.2d/%.2d/%.4d,%.2d:%.2d:%.2d,",*(temp_ch),*(temp_ch+1),*(int*)(temp_ch+3),*(temp_ch+4),*(temp_ch+5),*(temp_ch+6));

                n+=20;

                //strcpy((char*)&mySystem->MPG2->UART->Buffer_Rx[n],(char*)&mySystem->MPG2->UART->Buffer_Tx[40]);
                strcpy((char*)&mySystem->MPG2->UART->Buffer_Rx[n],(char*)&mySystem->MPG2->UART->Buffer_Tx[posBuffData]);

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

                //mySystem->MPG2->UART->Buffer_Tx[40]='\0';
                n=strlen((char*)mySystem->MPG2->UART->Buffer_Rx);

                // Stores register on I2C Memory 0 (addr:0x60000)
                mySystem->loggerSAT->loggingData(*mySystem->MPG2->Flowmeter[m]->getPointerWrite(),(char*)mySystem->MPG2->UART->Buffer_Rx, n+1, mySystem->Mem); // Almacena datos en memeoria EEPROM I2C sección logger Satelital.
                mySystem->Mem->setNewRegMPFlag();
                }
                break;

               // mySystem->loggerSAT->incNData();
////////////////////////////////
              case 1: // Format for GPRS DATA
                if(mySystem->loggerGPRS->isFlagStoreLogger())
                {
                  mySystem->loggerGPRS->clearFlagStoreLogger();
                  if((*mySystem->MPG2->Flowmeter[m]->getPointerWrite()==*mySystem->MPG2->Flowmeter[m]->getEEPROMstartAddr(m)))
                  {
                    mySystem->MPG2->UART->Buffer_Rx[n++]=mySystem->MPG2->get_ID_Rep();
                    mySystem->MPG2->UART->Buffer_Rx[n++]=',';

                    strncpy((char*)&mySystem->MPG2->UART->Buffer_Rx[n],mySystem->getSerialNumber(),10);
                    n+=10;
                    mySystem->MPG2->UART->Buffer_Rx[n++]=',';

                    mySystem->MPG2->UART->Buffer_Rx[n++]='0';
                    mySystem->MPG2->UART->Buffer_Rx[n++]='0';
                    mySystem->MPG2->UART->Buffer_Rx[n++]='0';
                    mySystem->MPG2->UART->Buffer_Rx[n++]=',';


                    j=0;
                    temp_ch=mySystem->getIDFMorIS();
                    while(temp_ch[j]!='\0' && j<3)
                    {
                      mySystem->MPG2->UART->Buffer_Rx[n++]=temp_ch[j++];
                    }
                    mySystem->MPG2->UART->Buffer_Rx[n++]=',';

                    j=0;
                    temp_ch=mySystem->getRFC();
                    while(temp_ch[j]!='\0' && j<20)
                    {
                      mySystem->MPG2->UART->Buffer_Rx[n++]=temp_ch[j++];
                    }
                    mySystem->MPG2->UART->Buffer_Rx[n++]=',';

                    j=0;
                    temp_ch=mySystem->MPG2->getSerialNumberFM(); // Numero de serie lógico del medidor conectado
                    while(temp_ch[j]!='\0' && j<12)
                    {
                      mySystem->MPG2->UART->Buffer_Rx[n++]=temp_ch[j++];
                    }
                    mySystem->MPG2->UART->Buffer_Rx[n++]=',';

                    j=0;
                    temp_ch=mySystem->getExternalSerialNoFlowMet(); // Numero de serie externo del medidor (No. de serie externo)
                    while(temp_ch[j]!='\0' && j<12)
                    {
                      mySystem->MPG2->UART->Buffer_Rx[n++]=temp_ch[j++];
                    }
                    mySystem->MPG2->UART->Buffer_Rx[n++]=',';

                    j=0;
                    temp_ch=mySystem->getLocLatitude();
                    while(temp_ch[j]!='\0' && j<12)
                    {
                      mySystem->MPG2->UART->Buffer_Rx[n++]=temp_ch[j++];
                    }
                    mySystem->MPG2->UART->Buffer_Rx[n++]=',';

                    j=0;
                    temp_ch=mySystem->getLocLongitude();
                    while(temp_ch[j]!='\0' && j<12)
                    {
                      mySystem->MPG2->UART->Buffer_Rx[n++]=temp_ch[j++];
                    }

                    response=*mySystem->MPG2->Flowmeter[m]->getIDVariable();  // 0x01 registro normal, 0x02 registro de alama de puerta abierta, 0x03 registro de solicitud hecho por la Unidad Verificadora

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

                  /*sprintf((char*)&mySystem->MPG2->UART->Buffer_Rx[n],"%.2d/",*(temp_ch));
                  n+=4;
                  sprintf((char*)&mySystem->MPG2->UART->Buffer_Rx[n],"%.4d,%.2d:%.2d:%.2d,",*(int*)(temp_ch+3),*(temp_ch+4),*(temp_ch+5),*(temp_ch+6));
                  *temp_ch=mySystem->MPG2->UART->Buffer_Rx[n+2];
                  sprintf((char*)&mySystem->MPG2->UART->Buffer_Rx[n-1],"%.2d/",*(temp_ch+1));
                  mySystem->MPG2->UART->Buffer_Rx[n+2]=*temp_ch;
                  n+=14;*/

                  // Date format YYYYMMDD,hhmmss
                  sprintf((char*)&mySystem->MPG2->UART->Buffer_Rx[n],"%.4d%.2d%.2d,%.2d%.2d%.2d,",*(int*)(temp_ch+3),*(temp_ch+1),*(temp_ch),*(temp_ch+4),*(temp_ch+5),*(temp_ch+6));
                  n+=16;

                  response=*mySystem->MPG2->Flowmeter[m]->getIDVariable();
                  sprintf((char*)&mySystem->MPG2->UART->Buffer_Rx[n],"%.1d,",response);
                  n+=2;

                  //strcpy((char*)&mySystem->MPG2->UART->Buffer_Rx[n],(char*)&mySystem->MPG2->UART->Buffer_Tx[40]);
                  strcpy((char*)&mySystem->MPG2->UART->Buffer_Rx[n],(char*)&mySystem->MPG2->UART->Buffer_Tx[posBuffData]);

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


                  if(*mySystem->ModemGPRS->getFlags_reqInfo() & BIT3) // Solicitud por la unidad verificadora?
                  {
                    mySystem->ModemGPRS->flags_req_info&=~BIT3;
                    mySystem->ModemGPRS->flags_req_info|=BIT4; // Habilita bandera de envío de datos via GPRS del registro solicitado por la Unidad Verificadora
                    mySystem->MPG2->UART->Buffer_Rx[n++]=',';
                    j=0;
                    temp_ch=mySystem->getIDUV();
                    while(temp_ch[j]!='\0' && j<6)
                    {
                      mySystem->MPG2->UART->Buffer_Rx[n++]=temp_ch[j++];
                    }

                  }

                  mySystem->MPG2->UART->Buffer_Rx[n++]=0x0D;
                  mySystem->MPG2->UART->Buffer_Rx[n++]=0x0A;
                  mySystem->MPG2->UART->Buffer_Rx[n++]='\0';

                  //mySystem->MPG2->UART->Buffer_Tx[40]='\0';
                  mySystem->MPG2->UART->Buffer_Tx[posBuffData]='\0';
                  n=strlen((char*)mySystem->MPG2->UART->Buffer_Rx);
////////////////////////////////
                  // Starts to store registers in GPRS logger I2C EEPROM (From 0x61000 to 0x62FFF)
                  if((*mySystem->MPG2->Flowmeter[m]->getPointerWrite()+n)<*mySystem->MPG2->Flowmeter[m]->getEEPROMstartAddr(m+1))
                  {
                    if(n<=128)
                    {
                      mySystem->loggerGPRS->loggingData(*mySystem->MPG2->Flowmeter[m]->getPointerWrite(),(char*)mySystem->MPG2->UART->Buffer_Rx, n, mySystem->Mem); // Almacena datos en memeoria EEPROM I2C sección logger Satelital.
                      mySystem->MPG2->Flowmeter[m]->addPointerWrite(n);
                    }
                    else
                    {
                      mySystem->loggerGPRS->loggingData(*mySystem->MPG2->Flowmeter[m]->getPointerWrite(),(char*)&mySystem->MPG2->UART->Buffer_Rx[0], 128, mySystem->Mem); // Almacena datos en memeoria EEPROM I2C sección logger Satelital.
                      mySystem->MPG2->Flowmeter[m]->addPointerWrite(128);
                      mySystem->loggerGPRS->loggingData(*mySystem->MPG2->Flowmeter[m]->getPointerWrite(),(char*)&mySystem->MPG2->UART->Buffer_Rx[128], n-128, mySystem->Mem); // Almacena datos en memeoria EEPROM I2C sección logger Satelital.
                      mySystem->MPG2->Flowmeter[m]->addPointerWrite(n-128);
                    }
                    mySystem->loggerGPRS->incNData();
                  }
                } //ends if statement
                break;
              }
            }

          mySystem->saveToEEPROM();
          mySystem->MPG2->requestData=false;
          mySystem->MPG2->UART->off_RS_232();
          mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
          mySystem->MPG2->reInitComms();
          mySystem->RGBLed->offAll();
          mySystem->Reloj->startReloj();

          break;
          ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    case 1:// Finaliza comunicaciones en caso que se haya recibido adecuadamente la información del medidor
//          mySystem->ModemGPRS->counterTimeout=0;
//          mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
          mySystem->ModemGPRS->flags_req_info&=~BIT3; // Limpia bandera si se requirió solicitud de datos por la Unidad Verificadora

          mySystem->MPG2->requestData=false;
          mySystem->MPG2->UART->off_RS_232();
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
/*                Rutina de interrupción Tx para módulo Bluetooth                 */
/**********************************************************************************/
#pragma vector=USART0TX_VECTOR
__interrupt void intUART0Tx(void)
{
  int x;
  LPM3_EXIT;
  if(mySystem->ModemGPRS->UART->statePort&0x02)
  {
    mySystem->ModemGPRS->UART->dataPtrTx++;
    if ((mySystem->ModemGPRS->UART->dataPtrTx) < mySystem->ModemGPRS->UART->strTxLen)
    {
      //if(mySystem->ModemGPRS->UART->isBT_RTSUp())
      //{
      x=0;
      while(mySystem->ModemGPRS->UART->isBT_RTSUp() && x<=20000) // Evaluates RTS signal form BT
      {
        x++;
      }
      //}
      if(x<20000)
      {
        U0TXBUF=mySystem->ModemGPRS->UART->Buffer_Tx_Start[mySystem->ModemGPRS->UART->dataPtrTx];
      }
      else
      {
        mySystem->ModemGPRS->turnOFF_VCC_GPRS();
        mySystem->restart_TX_GPRS_BT();

        /*mySystem->ModemGPRS->UART->statePort&=~0x02;
        mySystem->ModemGPRS->counterTimeout=0;
        mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
        mySystem->MPG2->requestData=false;  // Finaliza transmisiones
        mySystem->ModemGPRS->UART->off_RS_232();
        mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
        mySystem->RGBLed->offAll();
        //mySystem->saveToEEPROM();
        mySystem->Reloj->startReloj();*/
      }
    }
    else
    {
      mySystem->ModemGPRS->UART->disableTXInterrupt();

      if ((mySystem->ModemGPRS->UART->dataPtrTx) == mySystem->ModemGPRS->UART->strTxLen)
      { while(!(U0TCTL&TXEPT))
        {
        }
        mySystem->ModemGPRS->turnOFF_VCC_GPRS();
        mySystem->restart_TX_GPRS_BT();

       /* mySystem->ModemGPRS->UART->statePort&=~0x02;
        mySystem->ModemGPRS->counterTimeout=0;
        mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
        mySystem->MPG2->requestData=false;  // Finaliza transmisiones
        mySystem->ModemGPRS->UART->off_RS_232();
        mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
        mySystem->RGBLed->offAll();
        //mySystem->saveToEEPROM();
        mySystem->Reloj->startReloj();*/
        //mySystem->Reloj->startReloj();
        //mySystem->ModemGPRS->UART->myWDT->restartWdt();
        //mySystem->MPG2->UART->myWDT->restartWdt();
      }
    }
  }
}
/**********************************************************************************/
/*                       Comunicaciones UART0 por hardware                        */
/*                  Rutina de interrupción Rx para módulo GPRS                    */
/**********************************************************************************/
#pragma vector=USART0RX_VECTOR
__interrupt void intUART0Rx(void)
{
  LPM3_EXIT;

  TACTL&=~TAIE; // Deshabilita la interrupcion para incremento de reloj.

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
  //mySystem->MPG2->UART->myWDT->restartWdt();

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

  if(!mySystem->m_Fat->SDMem->isSPIEnabled())
  {
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
  //IFG2&=~URXIFG1;

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

  //P5OUT^=0x01;
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
              mySystem->flagsAlarms|=BIT3;
              //mySystem->ModemGPRS->OFFfromGPRSSession();
            }
            else
            {
              mySystem->ModemGPRS->counterTimeout=0;
              mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
              mySystem->ModemGPRS->UART->statePort&=~0x02;
              mySystem->ModemGPRS->stateGPRS&=~(BIT0|BIT1|BIT3|BIT5); // Deshabilita bandera de solicitud de comando AT
              mySystem->ModemGPRS->UART->UartError&=~0x01;
              mySystem->ModemGPRS->UART->clearRxBuffPointer(); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
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
              mySystem->flagsAlarms|=BIT2;
              //mySystem->ModemGPRS->OffModemGPRS();
            }
            else
            {
              mySystem->ModemGPRS->counterTimeout=0;
              mySystem->ModemGPRS->timerReset->DisableTBCCTL0_IE();
              mySystem->ModemGPRS->UART->statePort&=~0x02;
              mySystem->ModemGPRS->stateGPRS&=~(BIT0|BIT1|BIT3|BIT5); // Deshabilita bandera de solicitud de comando AT
              mySystem->ModemGPRS->UART->UartError&=~0x01;
              mySystem->ModemGPRS->UART->clearRxBuffPointer(); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
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

  bool alarmGPRS=false;

  mySystem->RGBLed->changeStateOnLED();

  if(!mySystem->m_btnProg->isProgFlagActivated())
  {
    if(mySystem->RGBLed->isTimeToLighting())
    {
      if(mySystem->VLevelBatt->getVpot()>=Lev_Disch_Comm)
      {
        if(*mySystem->loggerGPRS->getPNDataBCD())
        {
          mySystem->RGBLed->onMagenta();
        }
        else
        {
          mySystem->RGBLed->onOnlyGreen();
        }
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
      if(*mySystem->loggerGPRS->getPNDataBCD())
      {
        mySystem->Reloj->stopReloj();
        mySystem->MPG2->requestData=true;
        mySystem->ModemGPRS->counterTimeout=0;
        mySystem->ModemGPRS->timerReset->EnableTBCCTL0_IE();
        mySystem->flagsAlarms|=BIT1;
        //mySystem->ModemGPRS->OnModemGPRS();
        //mySystem->ModemGPRS->connectGPRS();
      }
      else
      {
        mySystem->MPG2->requestData=false;
      }
   }

  // Lineas de incremento de reloj interno
  ////if(!mySystem->menuSelection)
  ////{
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

  ////}
  ////else
  ////{
  ////   mySystem->Reloj->incClock(1);
  ////}

  // Fin de lineas para reloj interno

  // Lineas para data logger interno y para reporte satelital

/*  if(mySystem->loggerSAT->isEnabledLogg())
  {
    mySystem->loggerSAT->incCounterSmplTime();
  }
*/

  if(mySystem->MPG2->requestData==false && !mySystem->m_btnProg->isProgFlagActivated() && !(*mySystem->ModemGPRS->getFlags_reqInfo() & BIT2) && !(*mySystem->ModemGPRS->getFlags_reqInfo() & BIT3) && !(*mySystem->ModemGPRS->getFlags_reqInfo() & BIT4))
  {
    //if(mySystem->MPG2->requestData==false && mySystem->loggerSAT->isTimeSampleOn(mySystem->Reloj))
    if(mySystem->loggerSAT->isTimeSampleOn(mySystem->Reloj))
    {
      mySystem->loggerSAT->setFlagStoreLogger();  // Alta para permitir almacenar el evento en en logger Interno de la telemetría
    }
    else
    {
      mySystem->loggerSAT->clearFlagStoreLogger();
    }

    //if(mySystem->MPG2->requestData==false && mySystem->loggerGPRS->isTimeSampleOn(mySystem->Reloj))
    if(mySystem->loggerGPRS->isTimeSampleOn(mySystem->Reloj))
    {
      mySystem->loggerGPRS->setFlagStoreLogger(); // Alta para permitir almacenar el evento en en logger para reportes GPRS
    }
    else
    {
      mySystem->loggerGPRS->clearFlagStoreLogger();
    }

    if(mySystem->loggerSAT->isFlagStoreLogger() || mySystem->loggerGPRS->isFlagStoreLogger())
    {
      if(mySystem->loggerSAT->isEnabledLogg())
      {
        mySystem->loggerSAT->copyDateTime(mySystem->Reloj);
        mySystem->MPG2->requestData=true;
        mySystem->MPG2->set_ID_Rep(0x32);
        mySystem->MPG2->Flowmeter[0]->setIDVariable(0x01); // Establece la variable como un registro normal de reporte
        mySystem->MPG2->Flowmeter[1]->setIDVariable(0x01); // Establece la variable como un registro normal de reporte

        if(mySystem->loggerGPRS->getNDataBCD()==0xFF)
        {
          mySystem->loggerGPRS->clearNData();
        }
        mySystem->Reloj->stopReloj();

        mySystem->MPG2->clearCommCounter();
        mySystem->MPG2->UART->myWDT->restartWdt();

        mySystem->MPG2->UART->commTries=0;
        mySystem->flagsAlarms|=BIT0;
        //mySystem->MPG2->sendFrameToMPG2();
      }
    }
  }
  else
  {
    mySystem->loggerSAT->clearFlagStoreLogger();
  }


  // Fin de lineas para data logger de logger interno y para reporte satelital

  // Lineas para detección de cumplimiento de horario de repote vía GPRS
  //if(*mySystem->loggerSAT->getPNDataBCD() && mySystem->MPG2->requestData==false && mySystem->VLevelBatt->getVpot()>=Lev_Disch_Comm && !mySystem->m_btnProg->isProgFlagActivated() && !mySystem->Mem->getNewRegMPFlag()) // Existe permiso para solicitar comunicaciones y el voltaje de la batería se encuentra en condicones para comunicar?
  if(mySystem->MPG2->requestData==false && mySystem->VLevelBatt->getVpot()>=Lev_Disch_Comm && !mySystem->m_btnProg->isProgFlagActivated() && !mySystem->Mem->getNewRegMPFlag())// Existe permiso para solicitar comunicaciones y el voltaje de la batería se encuentra en condicones para comunicar?
  {
    if(!(*mySystem->ModemGPRS->getFlags_reqInfo() & BIT4))
    {
      alarmGPRS=mySystem->Reloj->compareRegAlarm();
    }
    else
    {
      mySystem->ModemGPRS->flags_req_info&=~BIT4;
      alarmGPRS=true;
    }
    //if(mySystem->Reloj->compareRegAlarm() || (*mySystem->ModemGPRS->getFlags_reqInfo() & BIT4)) // Genera Alarma?
    if(alarmGPRS) // Genera Alarma?
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
                mySystem->flagsAlarms|=BIT3;
                //mySystem->ModemGPRS->OFFfromGPRSSession();
           }
           else
           {// Se ejecuto normalmemte al iniciar por primera vez el sistema o restetearlo (El modem está encendido)
             mySystem->flagsAlarms|=BIT2;
             //mySystem->ModemGPRS->OffModemGPRS();
           }
        }
        else
        {
          if(mySystem->ModemGPRS->isModemGPRSOn())
          {
            mySystem->ModemGPRS->stateGPRS|=BIT4;  // Habilita bandera de apagar el sistema
            mySystem->flagsAlarms|=BIT2;
            //mySystem->ModemGPRS->OffModemGPRS();
          }
          else
          {
            if(*mySystem->loggerGPRS->getPNDataBCD())
            {
              mySystem->flagsAlarms|=BIT1;
              //mySystem->ModemGPRS->OnModemGPRS();
              //mySystem->ModemGPRS->connectGPRS();
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
    if(mySystem->door->evalIsDoorOpen() || (*mySystem->ModemGPRS->getFlags_reqInfo() & BIT2))
    {// Puerta Abierta
      mySystem->door->incCounterTime();
      if(mySystem->door->isCounterFull() || (*mySystem->ModemGPRS->getFlags_reqInfo() & BIT2))
      { // Generar mensaje de alerta de puerta y almacenarlo en el logger de registros
        if(!mySystem->door->getDoorStateFlg() || (*mySystem->ModemGPRS->getFlags_reqInfo() & BIT2))
        {
          mySystem->loggerSAT->setFlagStoreLogger();  // Alta para permitir almacenar el evento en en logger Interno de la telemetría
          mySystem->loggerGPRS->setFlagStoreLogger(); // Alta para permitir almacenar el evento en en logger para reportes GPRS

          mySystem->MPG2->set_ID_Rep(0x32);

          if(*mySystem->ModemGPRS->getFlags_reqInfo() & BIT2)
          {
            mySystem->ModemGPRS->flags_req_info&=~BIT2;
            mySystem->ModemGPRS->flags_req_info|=BIT3;
            mySystem->MPG2->Flowmeter[0]->setIDVariable(0x03);
            mySystem->MPG2->Flowmeter[1]->setIDVariable(0x03); // Establece la variable como un registro de reporte normal a solicutd de una Unidad Verificadora
            mySystem->MPG2->restartWRPointers(0x0000);
            mySystem->loggerSAT->clearNData();
          }
          else
          {
            mySystem->MPG2->Flowmeter[0]->setIDVariable(0x02); // Establece la variable como un registro de reporte de error o alarma por puerta abierta
            mySystem->MPG2->Flowmeter[1]->setIDVariable(0x02); // Establece la variable como un registro de reporte de error o alarma por puerta abierta
            mySystem->door->setDoorOpenStateFlg();
          }

          mySystem->loggerSAT->copyDateTime(mySystem->Reloj);
          mySystem->MPG2->requestData=true;

          if(mySystem->loggerGPRS->getNDataBCD()==0xFF)
          {
            mySystem->loggerGPRS->clearNData();
          }
          mySystem->Reloj->stopReloj();
          mySystem->MPG2->clearCommCounter();
          mySystem->MPG2->UART->myWDT->restartWdt();
          //mySystem->MPG2->sendFrameToMPG2(mySystem->MPG2->getCommCounter()); // New Line
          mySystem->MPG2->UART->commTries=0;
          mySystem->flagsAlarms|=BIT0;
          //mySystem->MPG2->sendFrameToMPG2(); // New Line
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

  // Inicio de lineas de evaluación de tiempo para obtener el voltaje de la batería
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

 if(mySystem->m_btnProg->isButtonPressed())
 {
   if(mySystem->MPG2->requestData==false)
   {
    mySystem->ModemGPRS->clearValidatedPasswordFlag();
    //mySystem->MPG2->setEncryptionMode();
    //mySystem->MPG2->UART->on_RS_232();
    mySystem->m_btnProg->setProgFlag();
   }
   /*else
   {
     // Reset módulo Bluetooth
     //mySystem->ModemGPRS->BL652->resetProcedure();
   }*/
 }
/*
 mySystem->LCD->onBackLight();
 mySystem->LCD->clearBLCounter();

 if(mySystem->KBoard->KBFlags&0x01)
 {
  mySystem->setMenu(mySystem->KBoard->getKeyPushed());
 }
*/
 P1IFG=0x00;
}
/**********************************************************************************/
/*                           Interrupción Puerto 1                                */
/*                            Rutina para teclado                                 */
/**********************************************************************************/
#pragma vector=PORT2_VECTOR
__interrupt void PORT2ISR (void)
{
 LPM3_EXIT;
/*
 mySystem->ModemGPRS->UART->disableTXInterrupt();
 while(mySystem->ModemGPRS->UART->isBT_RTSUp())
 {}
 IFG1&=~UTXIFG0;
 mySystem->ModemGPRS->UART->enableTXInterrupt();
 IFG1|=UTXIFG0;
 //mySystem->ModemGPRS->UART->setBT_CTS();
 //mySystem->ModemGPRS->UART->clearBT_CTS();
 //U0TXBUF='\r';

 P2IFG=0x00;
 */
}
/**********************************************************************************/
/*                              Interrupción DMA                                  */
/*                            Rutina para TX UARTS                                */
/**********************************************************************************/
#pragma vector=DACDMA_VECTOR
__interrupt void UARTS_O_1(void)
{
  LPM3_EXIT;

  int x;
  char* p_char;
  unsigned long int k;

  if(DMA0CTL&DMAIFG)
  {
    /*if(*mySystem->ModemGPRS->getFlags_reqInfo() & BIT2) // Se solicitó reqerimiento de envío de reporte de prueba generado por la Unidad Verfificadora?
    {
      mySystem->ModemGPRS->BL652->resetProcedure();
    }*/

    mySystem->MPG2->UART->myWDT->restartWdt();
  }

  //if(!mySystem->MPG2->getFlagTXExt())
  if(!mySystem->ModemGPRS->getFlagTXExt())
  {

    if(DMA2CTL&DMAIFG)
    {
      if(mySystem->m_Fat->SDMem->isSPIEnabled())
      {
        for(x=0;x<10;x++)   // Adding time to dealay the disable signal (CS from SD Memory)
        {_NOP();}
        P5DIR&=~BIT0;   // Up signal

        if(mySystem->m_Fat->SDMem->isFlagStartTxCommand()) // Finalizó envío de comando solicitado para la memoria SD?
        {
          mySystem->m_Fat->SDMem->setFlagEndTxCommand();

          if(!(*mySystem->m_Fat->SDMem->getCommCounter())) // Si el comando anterior fué el tiempo de inicialización de la memoria SD (envio de 80 ciclos de CK)
          {
            mySystem->m_Fat->SDMem->clearFlagsRXTXCommand();
            mySystem->m_Fat->SDMem->incCommCounter();
            mySystem->m_Fat->SDMem->writeConfig();
            mySystem->m_Fat->SDMem->sendFrameToMemorySPI(); // Envía el comando siguiente
          }
          else
          { // Si es un comando diferente del de incialización.
            // Envía señal para leer datos de la memoria
            mySystem->m_Fat->SDMem->writeReadConfig();
            mySystem->m_Fat->SDMem->sendFrameToMemorySPI();
          }
        }
        else
        {
          if(mySystem->m_Fat->SDMem->isFlagStartRxCommand()) // Finalizó envío de pulsos para lectura de memeoria SD?
          {
            mySystem->m_Fat->SDMem->setFlagEndRxCommand();
            switch(mySystem->m_Fat->SDMem->getWorkProcess())
            {
            case BIT0: // Initialize
                    switch(*mySystem->m_Fat->SDMem->getCommCounter())
                    {
                    case 6:
                            if(mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[2]!=0x00)
                            {
                              if(mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[2]==0xFF && mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[3]==0xFF)
                              {// SD Card memory not installed
                                mySystem->m_Fat->SDMem->clearCommTries();
                                //mySystem->m_Fat->SDMem->setCommCounter(mySystem->m_Fat->SDMem->getEndCommand());
                              }
                              else
                              {
                                mySystem->m_Fat->SDMem->incCommTries();
                                if(*mySystem->m_Fat->SDMem->getCommTries()>=200)
                                {
                                  // Finalizar reintentos y  marcar el tipo de error de la memoria
                                  mySystem->m_Fat->SDMem->clearCommTries();
                                  mySystem->m_Fat->SDMem->setCommCounter(mySystem->m_Fat->SDMem->getEndCommand());
                                }
                                else
                                {
                                  mySystem->m_Fat->SDMem->setCommCounter(mySystem->m_Fat->SDMem->getCommCounter()-2);
                                }
                                mySystem->m_Fat->SDMem->p_readData=(char*)&mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[0];
                              }
                            }
                            else
                            {
                              // Memoria en estado libre para leer o escribir
                              mySystem->m_Fat->SDMem->clearCommTries();
                              mySystem->m_Fat->SDMem->clearWorkProcess();
                              mySystem->m_Fat->SDMem->p_readData=(char*)&mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[0];
                            }
                            mySystem->m_Fat->SDMem->clearWorkProcess();
                            break;
                    case 10: _NOP(); break;
                    case 5: _NOP(); break;
                    case 4: _NOP(); break;
                    case 3: _NOP(); break;
                    case 2: _NOP(); break;
                    case 1: _NOP(); break;
                    case 0: _NOP(); break;
                    default: break;
                    }
                    break;
            case BIT1: // Read Memory Data
                    switch(*mySystem->m_Fat->SDMem->getCommCounter())
                    {
                    case 6:
                            if(mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[2]!=0x00)
                            {
                              mySystem->m_Fat->SDMem->incCommTries();
                              if(*mySystem->m_Fat->SDMem->getCommTries()>=200)
                              {
                                // Finalizar reintentos y  marcar el tipo de error de la memoria
                                mySystem->m_Fat->SDMem->clearCommTries();
                                mySystem->m_Fat->SDMem->setCommCounter(mySystem->m_Fat->SDMem->getEndCommand());
                              }
                              else
                              {
                                mySystem->m_Fat->SDMem->setCommCounter(mySystem->m_Fat->SDMem->getCommCounter()-2);
                              }
                            }
                            else
                            {
                              // Memoria en estado libre para leer o escribir
                              mySystem->m_Fat->SDMem->clearCommTries();
                            }
                            break;
                    case 7: // Ensamblar la trama con la dirección a leer
                            _NOP();
                            break;
                    case 5: _NOP(); break;
                    case 100: // Agregar código para regresar el apuntador al primer dato leeido
                            x=0;
                            while(mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[x]!=0xFE && x<50)
                            {
                              x++;
                            }
                            if(x<50)
                            {
                              mySystem->m_Fat->SDMem->p_readData=(char*)&mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[x+1];
                            }
                            mySystem->m_Fat->SDMem->clearWorkProcess();
                            break;
                    default: break;
                    }
                    break;
            case BIT2: // Write Memory Data
                    switch(*mySystem->m_Fat->SDMem->getCommCounter())
                    {
                    case 6:
                            if(mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[2]!=0x00)
                            {
                              mySystem->m_Fat->SDMem->incCommTries();
                              if(*mySystem->m_Fat->SDMem->getCommTries()>=200)
                              {
                                // Finalizar reintentos y  marcar el tipo de error de la memoria
                                mySystem->m_Fat->SDMem->clearCommTries();
                                mySystem->m_Fat->SDMem->setCommCounter(mySystem->m_Fat->SDMem->getEndCommand());
                              }
                              else
                              {
                                mySystem->m_Fat->SDMem->setCommCounter(mySystem->m_Fat->SDMem->getCommCounter()-2);
                              }
                            }
                            else
                            {
                              // Memoria en estado libre para leer o escribir
                               mySystem->m_Fat->SDMem->clearCommTries();
                            }
                            break;
                    case 9:
                            if(mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[2]==0x01 && mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[3]==0x00)
                            {
                              // ends process
                              mySystem->m_Fat->SDMem->clearCommTries();
                              mySystem->m_Fat->SDMem->clearWorkProcess();
                            }
                            else
                            {
                              // retry command 9
                              mySystem->m_Fat->SDMem->incCommTries();
                              if(*mySystem->m_Fat->SDMem->getCommTries()>=200)
                              {
                                mySystem->m_Fat->SDMem->clearCommTries();
                              }
                              else
                              {
                                mySystem->m_Fat->SDMem->setCommCounter(mySystem->m_Fat->SDMem->getCommCounter()-2);
                              }
                            }
                            break;
                    case 8: // Ensamblar la trama con la dirección a escribir
                            _NOP();
                            break;
                    case 5: _NOP(); break;
                    case 101: // Agregar código para validar que el bloque fué correctamente escrito en memoria
                            if((!(mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[1]&BIT4)) && (mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[1]&BIT0) && (!(mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[1]&BIT1)) && (mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[1]&BIT2) && (!(mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[1]&BIT3)))
                            {
                              // Esritura del bloque de memoria exitoso y finalizar proceso
                               /*for(x=0;x<20000;x++)
                               {
                                 _NOP();
                               }*/
                               mySystem->m_Fat->SDMem->clearCommTries();
                               //mySystem->m_Fat->SDMem->clearWorkProcess();
                            }
                            else
                            {
                              mySystem->m_Fat->SDMem->clearCommTries();
                              /*
                              mySystem->m_Fat->SDMem->incCommTries();
                              if(*mySystem->m_Fat->SDMem->getCommTries()>=50)
                              {
                                // Finalizar reintentos y  marcar el tipo de error de la memoria
                                mySystem->m_Fat->SDMem->clearCommTries();
                              }
                              else
                              {
                                mySystem->m_Fat->SDMem->setCommCounter(mySystem->m_Fat->SDMem->getCommCounter()-4);
                              }
                              */
                            }
                             break;
                    /*case 102:
                            if(mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[1]==0xFF)
                            {
                              // Proceso de espera de escitura de memoria finalizado
                              mySystem->m_Fat->SDMem->clearCommTries();
                             // mySystem->SDMem->clearWorkProcess();
                            }
                            else
                            {
                              mySystem->m_Fat->SDMem->incCommTries();
                              if(*mySystem->m_Fat->SDMem->getCommTries()>=250)
                              {
                                // Finalizar reintentos y  marcar el tipo de error de la memoria
                                mySystem->m_Fat->SDMem->clearCommTries();
                              }
                              else
                              {
                                mySystem->m_Fat->SDMem->setCommCounter(mySystem->m_Fat->SDMem->getCommCounter()-1);
                              }
                            }
                            mySystem->m_Fat->SDMem->clearWorkProcess();*/
                    default: break;
                    }
                    break;
            default:break;

            }

            mySystem->m_Fat->SDMem->incCommCounter();
            mySystem->m_Fat->SDMem->clearFlagsRXTXCommand();

            if(mySystem->m_Fat->SDMem->getCommCounter()>=mySystem->m_Fat->SDMem->getEndCommand())
            {// Fin de envío de comandos

                /*
                mySystem->SDMem->setFlagEndTxCommand();
                mySystem->SDMem->UART->strRxLen=0;
                mySystem->SDMem->UART->dataPtrRx=0;
                mySystem->SDMem->UART->clearRxBuffPointer();
                mySystem->SDMem->writeReadConfig();
                mySystem->SDMem->sendFrameToMemorySPI();

                mySystem->SDMem->UART->strRxLen=0;
                mySystem->SDMem->UART->dataPtrRx=0;
                mySystem->SDMem->UART->clearRxBuffPointer();
                mySystem->SDMem->writeConfig();
                mySystem->SDMem->sendFrameToMemorySPI();

              */
                mySystem->m_Fat->SDMem->clearWorkProcess();
                _NOP();
            }
            else
            { // Envía siguiente comando
              //IFG2&=~URXIFG1;
              /*mySystem->SDMem->UART->strRxLen=0;
              mySystem->SDMem->UART->dataPtrRx=0;
              mySystem->SDMem->UART->clearRxBuffPointer();
              */
            switch(mySystem->m_Fat->SDMem->getWorkProcess())
            {
              case BIT1:
                       switch(*mySystem->m_Fat->SDMem->getCommCounter())
                       {
                       case 7:// Set block to read
                                x=0;
                                p_char=(char*)&mySystem->m_Fat->SDMem->RD_Addr;
                                mySystem->m_Fat->SDMem->UART->SD_Buffer_Tx[x]=commandSPI[*mySystem->m_Fat->SDMem->getCommCounter()-1][x];
                                x++;
                                for(;x<5;x++)
                                {
                                  mySystem->m_Fat->SDMem->UART->SD_Buffer_Tx[x]=p_char[4-x];
                                }
                                mySystem->m_Fat->SDMem->UART->SD_Buffer_Tx[x]=0x95;
                                x++;
                                //mySystem->SDMem->RD_Addr++;
                                mySystem->m_Fat->SDMem->writeConfig();
                                mySystem->m_Fat->SDMem->sendFrameToMemorySPI();
                                break;
                       case 100:
                                mySystem->m_Fat->SDMem->setFlagEndTxCommand();
                                mySystem->m_Fat->SDMem->writeReadConfig();
                                mySystem->m_Fat->SDMem->sendFrameToMemorySPI();
                                break;
                       default:
                                mySystem->m_Fat->SDMem->writeConfig();
                                mySystem->m_Fat->SDMem->sendFrameToMemorySPI();
                                break;
                       }
                       break;
              case BIT2:
                       switch(*mySystem->m_Fat->SDMem->getCommCounter())
                       {
                       case 8:// Set block to write
                                x=0;
                                p_char=(char*)&mySystem->m_Fat->SDMem->WR_Addr;
                                mySystem->m_Fat->SDMem->UART->SD_Buffer_Tx[x]=commandSPI[*mySystem->m_Fat->SDMem->getCommCounter()-1][x];
                                x++;
                                for(;x<5;x++)
                                {
                                  mySystem->m_Fat->SDMem->UART->SD_Buffer_Tx[x]=p_char[4-x];
                                }
                                mySystem->m_Fat->SDMem->UART->SD_Buffer_Tx[x]=0x95;
                                x++;
                                //mySystem->SDMem->WR_Addr++;
                                mySystem->m_Fat->SDMem->writeConfig();
                                mySystem->m_Fat->SDMem->sendFrameToMemorySPI();
                                break;
                       case 101:      // Assemble data buffer to write in memory

                                x=0;

                                mySystem->m_Fat->SDMem->UART->SD_Buffer_Tx[x++]=0xFE;
                                  //mySystem->Mem->clearChangeFlagBank();
                               if(mySystem->Mem->getFlagWritingBckUpSD())
                               {
                                 //mySystem->Mem->clearFlagBckUpSD();
                                 mySystem->Mem->disableFlagWritingBckUpSD();
                                 k=addrBackupSDMem;
                                 for(x=0;x<4;x++)
                                 {
                                  mySystem->loggerSAT->readLoggedData((k+(x*128)), (char*)mySystem->m_Fat->SDMem->UART->SD_Buffer_Tx+(x*128)+1, mySystem->Mem);
                                 }
                               }
                               /*else
                               {
                                k=(*(unsigned long int*)mySystem->MPG2->Flowmeter[0]->getEEPROMstartAddr(0));

                                for(x=0;x<4;x++)
                                {
                                  mySystem->loggerSAT->readLoggedData((k+(x*128)), (char*)mySystem->m_Fat->SDMem->UART->SD_Buffer_Tx+(x*128)+1, mySystem->Mem);
                                }
                               }*/


                                mySystem->m_Fat->SDMem->writeConfig();
                                mySystem->m_Fat->SDMem->sendFrameToMemorySPI();
                                break;
                       /*case 102:
                                mySystem->m_Fat->SDMem->setFlagEndTxCommand();
                                mySystem->m_Fat->SDMem->writeReadConfig();
                                mySystem->m_Fat->SDMem->sendFrameToMemorySPI();
                                break;*/
                       default:
                                mySystem->m_Fat->SDMem->writeConfig();
                                mySystem->m_Fat->SDMem->sendFrameToMemorySPI();
                                break;
                       }
                       break;
              default:
                       mySystem->m_Fat->SDMem->writeConfig();
                       mySystem->m_Fat->SDMem->sendFrameToMemorySPI();
                       break;
            }


            }
          }
        }

      }
      else
      {
        mySystem->MPG2->UART->myWDT->restartWdt();
      }
    }
  }
  else
  {
    /*if(mySystem->m_btnProg->isProgFlagActivated())
    {
      mySystem->MPG2->UART->on_RS_232();
      mySystem->m_btnProg->setProgFlag();
      mySystem->m_btnProg->clearCounterSeconds();
    }*/
    //mySystem->MPG2->clearFlagTXExt();
    mySystem->ModemGPRS->clearFlagTXExt();
    mySystem->saveToEEPROM();
    mySystem->MPG2->requestData=false;
    mySystem->MPG2->reInitComms();
    mySystem->Reloj->setRTCOnIntTime(mySystem->Mem,false);
    mySystem->Reloj->startReloj();
  }
  DMA0CTL&=~DMAIFG;
  DMA2CTL&=~DMAIFG;

  if(DMA1CTL&DMAIFG)
  {
    DMA1CTL&=~DMAIFG;
  }
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
