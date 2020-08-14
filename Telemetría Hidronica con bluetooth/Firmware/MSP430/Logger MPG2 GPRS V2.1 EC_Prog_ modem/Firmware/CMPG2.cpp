#include "CDevice.h"

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"


#define Num_ser_flash  0x1000  // Dirección del apuntador a número de serie del medidor en la región FLASH de Datos del microcontrolador.
#define portEncrypt BIT2  // P1.2 Input to detect encryption function
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
CMPG2::CMPG2()
{
  char x;

  this->configEncrytionPort();

  for(x=0;x<NumMedidores;x++)
  {
    this->Flowmeter[x]=new CMedidor(x);
  }
  strcpy(this->serialNumber,"12020000");
  this->commCounter=0x00;
  this->nMedidores=0x01; //  1= 1 medidor, 2= 2 medidores...etc.
  this->ID_Rep=0x00;
  this->requestData=false;
  this->flagsTXExt=false;
  this->flagEncrypted=true; // Badera para encriptar=rue o no encriptar=false infromación
  UART=new CUart1();
}
/****************************************************************************************************/
CMPG2::~CMPG2()
{
}
/****************************************************************************************************/
#define DLE 0x10
#define STX 0x02
#define SRC 0x01
#define ETX 0x03

//const char* MilltronicsCommand[]={"F0\r\n2.71828\r\nP1\r\n3\r\nP5\r\n6\r\nF2\r\n/D0//D1//t1/",  // Nivel, Gasto, Volumen además configura distancia en mts y gasto en m^3/hr  y volumen en m^3
//                                         };

const char* MPG2Command[]={"HRID:",
                           "HRPT:",
                           "HRFL:",
                           "HRVL:",
                           "HRUM:",
                          };
/****************************************************************************************************/
// Implementación de Funciones.
/****************************************************************************************************/
void CMPG2::sendFrameToMPG2(int Command)
{
//  if(this->requestData==false)
//  {
     //this->requestData=true;
     int mylen=0;
//     this->commCounter=Command;
     mylen=this->assembleStringMPG2(&this->commCounter);
     this->UART->startTx((char *)this->UART->Buffer_Tx,mylen); // Envía la cadena de datos por el puerto serie.
//  }
}
/****************************************************************************************************/
void CMPG2::sendFrameTx(char* BufferTx, int lenght)
{
     int mylen=lenght;
     this->setEncryptionMode();
     this->encryptData((long int*)&keyWord, &BufferTx[0], lenght, true);  // Data encryption
     this->UART->startTx((char *)BufferTx,mylen); // Envía la cadena de datos por el puerto serie.
}
/****************************************************************************************************/
// El codigo evalúa sobre los separadores del formato de fecha y cantidad de dígitos del mes.
int CMPG2::evalMedidorRxString(char* BufferRx,int RxLength,char* SerNo,CLogger* m_Logger,CRTC_3029_I2C *Reloj,CEEPROM *memory,char* dns,char* ipPort, char *apnName,char *apnUser, char *apnPsw, char *idSP)
{
   int response=0x00;
   char *RxPos,*temp_ch1;
   int respLength=0;

   char* BufferTx=&BufferRx[0];
   int BufferRec[7];

   //if(RxLength>50)
   //{

   char m_CRC;

   if(this->requestData)
   {
    m_CRC=calcCRCMPG2(BufferRx,RxLength-4);
    if(m_CRC==this->getCRCbuffer(BufferRx, RxLength))
    {
      response=0x01;
    }
   }
    else
    {// Código para evaluar comandos externos para configuración de equipo

      m_CRC=calcCRCMPG2(BufferRx,RxLength-4);
      if(m_CRC==this->getCRCbuffer(BufferRx, RxLength))
      {
      RxPos=strstr(BufferRx,"H");
      if(RxPos)
      {
        if(this->evalSN(SerNo,RxPos+5))
        {
        RxPos++;
        switch(*RxPos++)
        {
        case 'W':
                  switch(*RxPos++)
                  {
                    case 'A':
                              switch(*RxPos)
                              {
                              case 'N': // Writes on Alarm Number Register OK
                                      sscanf(&BufferRx[16],"%2d",&BufferRec[0]);
                                      Reloj->setNAlarms((char)BufferRec[0]);
                                      sprintf(&BufferTx[5],"%s","OK;");
                                      respLength=8;
                                      break;
                              case 'R': // Writes one alarm hour register in the index specified (Nregister goes from 0 to 23) OK
                                      sscanf(&BufferRx[16],"%2d%2d%2d",&BufferRec[0],&BufferRec[1],&BufferRec[2]); // Nreg hh mm
                                      Reloj->setAlarm((char)BufferRec[0],(char)BufferRec[1],(char)BufferRec[2]);
                                      sprintf(&BufferTx[5],"%s","OK;");
                                      respLength=8;
                                      break;
                              default:
                                      break;
                              }
                              break;
                    case 'C':
                              switch(*RxPos)
                              {
                                case 'D': // Cambiar DNS & IP_PORT OK
                                          // HWCD:www.google.com,80
                                         RxPos+=2;
                                         RxPos=strstr(RxPos,";");
                                         if(RxPos)
                                         {
                                           RxPos++;
                                           temp_ch1=strstr(RxPos,";");
                                           BufferRec[0]=temp_ch1-RxPos;
                                           strncpy(dns,RxPos,BufferRec[0]);
                                           dns[BufferRec[0]]='\0';

                                           temp_ch1++;
                                           RxPos=strstr(temp_ch1,";");
                                           BufferRec[0]=RxPos-temp_ch1;
                                           strncpy(ipPort,temp_ch1,BufferRec[0]);
                                           ipPort[BufferRec[0]]='\0';

                                           sprintf(&BufferTx[5],"%s","OK;");
                                           respLength=8;
                                         }
                                         break;
                                case 'S':       // Cerrar sesión de configuración
                                         break;
                                default: break;
                              }
                              break;
                    case 'D':
                                switch(BufferRx[3])
                                {
                                case 'T': // Fecha y hora del sistema OK
                                         sscanf(&BufferRx[16],"%2d%2d%4d%2d%2d%2d%3d",(int*)&BufferRec[0],(int*)&BufferRec[1],(int*)&BufferRec[2],(int*)&BufferRec[3],(int*)&BufferRec[4],(int*)&BufferRec[5],(int*)&BufferRec[6]);
                                         Reloj->hexToBcdRTC(BufferRec);
                                         Reloj->setHour_Date(memory);
                                         Reloj->setRTCOnIntTime(memory,false);
                                         Reloj->setTimeZone(BufferRec[6]);
                                         sprintf(&BufferTx[5],"%s","OK;");
                                         respLength=8;
                                         break;
                                default: break;
                                }
                                break;
                    case 'L':
                                switch(BufferRx[3])
                                {
                                case 'T': // Establecer tiempo de recolección y estado del logger OK
                                         sscanf(&BufferRx[16],"%2d",(int*)&respLength);
                                         m_Logger->setIndexSampleTime(respLength);
                                         if(BufferRx[19]=='D')
                                         {m_Logger->disableLogg();}
                                         else
                                         {m_Logger->enableLogg();}
                                         sprintf(&BufferTx[5],"%s","OK;");
                                         respLength=8;
                                         break;
                                default: break;
                                }
                                break;
                    case 'O':
                              switch(*RxPos)
                              {
                                case 'S':
                                         break;
                                default: break;
                              }
                              break;
                    case 'S':
                              switch(*RxPos)
                              {
                                case 'P':       // Configurar APN, USUARIO y PASSWORD proveedor de servicio GPRS.OK
                                  // HWSP:internet.itelcel.com,webgprs,webgprs
                                         RxPos+=2;
                                         RxPos=strstr(RxPos,";");
                                         if(RxPos)
                                         {
                                           RxPos++;
                                           temp_ch1=strstr(RxPos,";");
                                           BufferRec[0]=temp_ch1-RxPos;
                                           strncpy(apnName,RxPos,BufferRec[0]);
                                           apnName[BufferRec[0]]='\0';

                                           temp_ch1++;
                                           RxPos=strstr(temp_ch1,";");
                                           BufferRec[0]=RxPos-temp_ch1;
                                           strncpy(apnUser,temp_ch1,BufferRec[0]);
                                           apnUser[BufferRec[0]]='\0';

                                           RxPos++;
                                           temp_ch1=strstr(RxPos,";");
                                           BufferRec[0]=temp_ch1-RxPos;
                                           strncpy(apnPsw,RxPos,BufferRec[0]);
                                           apnPsw[BufferRec[0]]='\0';

                                           *idSP=0x03;
                                           sprintf(&BufferTx[5],"%s","OK;");
                                           respLength=8;
                                         }
                                         break;
                                default: break;
                              }
                              break;

                    default: break;
                  }
                  break;
        case 'R':
                  switch(BufferRx[2])
                       {
                       case 'A':
                                switch(BufferRx[3])
                                {
                                case 'N': // Reads Alarm number register OK
                                        sprintf((char*)&BufferTx[5],"%.2d;",Reloj->getNAlarms());
                                        respLength=strlen(&BufferTx[0]);
                                        break;
                                 case 'R': // Reads one alarm hour register in the index specified (Nregister goes from 0 to 23) OK
                                      sscanf(&BufferRx[16],"%2d",&BufferRec[0]); // Nreg
                                      sprintf(&BufferTx[5],"%.2d%.2d;",Reloj->alarms[BufferRec[0]]->al_hour,Reloj->alarms[BufferRec[0]]->al_min); // Nreg hh mm
                                      respLength=strlen(&BufferTx[0]);
                                      break;
                                default:
                                        break;
                                }
                                break;
                       case 'C':
                                switch(BufferRx[3])
                                {
                                case 'D': // Read DNS & IP_PORT OK
                                         BufferTx[5]='\0';
                                         strcat(&BufferTx[0],dns);
                                         respLength=strlen(BufferTx);
                                         BufferTx[respLength++]=';';
                                         BufferTx[respLength]='\0';
                                         strcat(&BufferTx[0],ipPort);
                                         respLength=strlen(BufferTx);
                                         BufferTx[respLength++]=';';
                                         break;
                                default: break;
                                }
                                break;
                       case 'D':
                                switch(BufferRx[3])
                                {
                                case 'T': // Fecha y hora del sistema OK
                                         sprintf(&BufferTx[5],"%.2d%.2d%.4d%.2d%.2d%.2d%+.2d;",Reloj->day,Reloj->month,Reloj->year,Reloj->hour,Reloj->min,Reloj->seg,*Reloj->getTimeZone());
                                         respLength=strlen(&BufferTx[0]);
                                         //respLength=20;
                                         break;
                                default: break;
                                }
                                break;
                       case 'F':
                                switch(BufferRx[3])
                                {
                                case 'V':

                                         break;
                                default: break;
                                }
                                break;
                        case 'I':
                                switch(BufferRx[3])
                                {
                                case 'D': // Lee numero de serie con conformación del mismo OK
                                         for(respLength=0;respLength<10;respLength++)
                                         {
                                            BufferTx[5+respLength]=*(char*)(SerNo+respLength);
                                         }
                                         BufferTx[5+respLength]=';';
                                         respLength+=6;
                                         break;
                                default: break;
                                }
                                break;
                        case 'L':
                                switch(BufferRx[3])
                                {
                                case 'T': // Leer recolección y estado del logger OK
                                         sprintf(&BufferTx[5],"%.2d;%.2d;",*m_Logger->getIndexSampleTime(),*m_Logger->getTimeLoggerSatByIndex(*m_Logger->getIndexSampleTime()));
                                         if(m_Logger->isEnabledLogg())
                                         {BufferTx[11]='E';}
                                         else
                                         {BufferTx[11]='D';}
                                         BufferTx[12]=';';
                                         respLength=13;
                                         break;
                                default: break;
                                }
                                break;
                       case 'S':
                                switch(BufferRx[3])
                                {
                                case 'P':  // Read APN, USUARIO y PASSWORD proveedor de servicio GPRS. OK
//                                  char *apnName,char *apnUser, char *apnPsw, char *idSP)
                                         BufferTx[5]='\0';
                                         strcat(&BufferTx[0],apnName);
                                         respLength=strlen(BufferTx);
                                         BufferTx[respLength++]=';';
                                         BufferTx[respLength]='\0';
                                         strcat(&BufferTx[0],apnUser);
                                         respLength=strlen(BufferTx);
                                         BufferTx[respLength++]=';';
                                         BufferTx[respLength]='\0';
                                         strcat(&BufferTx[0],apnPsw);
                                         respLength=strlen(BufferTx);
                                         BufferTx[respLength++]=';';

                                         break;
                                default: break;
                                }
                                break;
                       default: break;
                       }
                  break;
        default:  break;
        }// fin switch
        }// fin if eval S/N
        else
        {
          switch(BufferRx[0])
          {
          case 'H':
                    switch(BufferRx[1])
                    {
                    case 'R':
                            switch(BufferRx[2])
                            {
                            case 'I':
                                      switch(BufferRx[3])
                                      {
                                      case 'D': // Lee numero de serie sin confirmación OK
                                              for(respLength=0;respLength<10;respLength++)
                                              {
                                                  BufferTx[5+respLength]=*(char*)(SerNo+respLength);
                                              }
                                              BufferTx[5+respLength]=';';
                                              respLength+=6;
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
        } //Fin else eval S/N
      }// Fin eval "H"
      }// Fin eval CRC Tx externo

    if(respLength)
    {
      BufferTx[4]=':';
      m_CRC=0;
      m_CRC=calcCRCMPG2(BufferRx,respLength);
      //m_CRC=this->m_CO3Generator->makeXORBuffer(BufferTx,respLength,l_CRC);
      //l_CRC++;
      sprintf((char *)&BufferTx[respLength], "%.2X%c%c",m_CRC,'\r','\n');
      respLength+=4;
      response=respLength;
    }
    }
   //}

   return response;
}
/****************************************************************************************************/
char CMPG2::calcCRCMPG2(char* xBuffer, int length)
{
  int CRC=0;
  for(int x=0; x<length; x++)
     CRC^=*xBuffer++;
  return CRC+1;
}
/****************************************************************************************************/
int CMPG2::assembleStringMPG2(int* Command)
{
  int myLen=0;

     strcpy((char*)&this->UART->Buffer_Tx[myLen],(char*)MPG2Command[*Command]);
     myLen=strlen((char*)&this->UART->Buffer_Tx[0]);
     strcpy((char*)&this->UART->Buffer_Tx[myLen],this->serialNumber);
     myLen+=8;
     this->UART->Buffer_Tx[myLen++]=';';
     this->UART->Buffer_Tx[myLen]='\0';
     myLen=strlen((char*)&this->UART->Buffer_Tx[0]);
     sprintf((char *)&this->UART->Buffer_Tx[myLen], "%.2X%c%c",calcCRCMPG2((char*)&this->UART->Buffer_Tx[0],myLen),0x0D,0x0A);
     myLen=strlen((char*)&this->UART->Buffer_Tx[0]);
     this->encryptData((long int*)&keyWord, (char*)&this->UART->Buffer_Tx[0], myLen, true);  // Data encryption
     return myLen;

}
/****************************************************************************************************/
/*int CItzyFlow::assembleString(int* Command)
{

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

}*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
void CMPG2::parsingData_Units(char* DataBuffer, char* BufferStore, int* NCommand)
{
   int x=0; //,y;
   int i=40;
   char *p_i;
   char *startBuffer=DataBuffer; // Apunta al inicio de Buffer_Rx
//   char *indexchr;

switch(*NCommand)
{
case 0: // Obtiene Número de serie del medidor
        strncpy(this->serialNumber,&startBuffer[5],8);
        BufferStore[i]='\0';
        break;
case 1: // Obtiene tipo de producto del medidor
        p_i=strchr(&BufferStore[i],'\0');
        strncpy(p_i,(char*)&this->serialNumber,8);
        p_i+=8;
        *p_i++=',';
        x=5;
        *p_i++=startBuffer[x++];
        *p_i++=startBuffer[x++];
        *p_i++=',';
        *p_i++='\0';
        break;
case 2: // Obtiene Gasto Instantáneo
        x=5;
        p_i=strchr(&BufferStore[i],'\0');
        *p_i++=startBuffer[x++];
        *p_i++=startBuffer[x++];
        switch(startBuffer[10])
        {
        case '4':
                  *p_i++=startBuffer[x++];
                  *p_i++=startBuffer[x++];
                  break;
        case '3':
                  *p_i++=startBuffer[x++];
                  *p_i++='.';
                  *p_i++=startBuffer[x++];
                  break;
        case '2':
                  *p_i++='.';
                  *p_i++=startBuffer[x++];
                  *p_i++=startBuffer[x++];
        default:  break;
        }
        *p_i++=',';
        *p_i++='\0';
        break;
case 3: // Obtiene Volumen Acumulado
        p_i=strchr(&BufferStore[i],'\0');
        strncpy(p_i,&startBuffer[5],12); // Dividorlo en servidor por 100 para ponerlo como litros, dividorlo por 100,000 para m^3
        p_i+=12;
        //*p_i++=0x0D;
        //*p_i++=0x0A;
        *p_i++='\0';
        break;
/*
        for(y=0;y<3;y++) // Hasta la cantidad de variables a obtener (Gasto, Volumen, Nivel)
        {
          indexchr=strchr(startBuffer,'\r'); // Fin de la cadena
          x=indexchr-startBuffer;

          switch(y)
          {
          case 0:    // Obtiene Nivel punto flotante mts
          case 1:    // Obtiene Gasto m^3/hr
                  strncpy(&BufferStore[i],startBuffer,x);
                  i+=x;
                  BufferStore[i++]=',';
                  break;
          case 2:    // Obtiene volumen m^3
                  indexchr=strchr(startBuffer,'.'); // detecta el caracter '.'
                  x=indexchr-startBuffer;
                  strncpy(&BufferStore[i],startBuffer,x);
                  i+=x;
                  BufferStore[i++]=0x0D;
                  BufferStore[i++]=0x0A;
                  BufferStore[i++]=0x00;
                  indexchr=strchr(startBuffer,'\r'); // Detecta de nuevo Fin de la cadena
                  break;
          default:
                  break;
          }
          startBuffer=indexchr+2;
        }
        break;
*/
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

int CMPG2::timeoutTxRx(char* SerNo,CLogger* m_Logger,CRTC_3029_I2C *Reloj,CEEPROM *memory,char* dns,char* ipPort, char *apnName,char *apnUser, char *apnPsw, char *idSP)
{
  int resp=0;

  if(this->UART->statePort&0x02)  // Comunicaciones Tx o Rx del medidor Milltronics?
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
        this->encryptData((long int*)&keyWord, (char*)&this->UART->Buffer_Rx[0], this->UART->strRxLen, false); // Data decryption
        resp=this->evalMedidorRxString((char*)&this->UART->Buffer_Rx[0],this->UART->strRxLen,SerNo,m_Logger,Reloj,memory,dns,ipPort,apnName,apnUser,apnPsw,idSP);
        if (resp==0x01) // Se recibió la respuesta de manera correcta del medidor?
        {
          this->parsingData_Units((char*)&this->UART->Buffer_Rx[0], (char*)&this->UART->Buffer_Tx[0], &this->commCounter);

          this->incCommCounter();
          this->UART->extraTimeCounter=0;
          this->UART->commTries=0;
          this->UART->clearRxBuffPointer(this->UART); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
          this->UART->strRxLen=0;
          this->UART->UartError&=~0x01;

          if(this->getCommCounter()>4)
          {// Finaliza las comunicaciones una vez que se obtuvo la información del medidor
            this->clearCommCounter();
            resp=2;
          }
          else // De otro modo se envía el siguiente comando para el medidor MPG2
          {
            this->UART->myWDT->restartWdt();
            this->sendFrameToMPG2(this->commCounter);
            resp=0;
          }

        }
        else  //No se recibió la respuesta correcta o falta información.
        {
          if(resp)
          {
            this->setFlagTXExt();
            this->sendFrameTx((char*)&this->UART->Buffer_Rx[0], resp);
          }
          else
          {
             if(this->requestData) // Se pidió info al medidor?
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
                    this->reInitComms();
                    resp=1;
                  }
                  else // Retransmite último comando solicitado.
                  {
                    this->UART->myWDT->restartWdt();
                    this->sendFrameToMPG2(this->commCounter);
                  }
                }
             }
             else // Se recibió cadena sin algun significado
             {
               this->reInitComms();
               resp=1;
             }
          }
        }
      }
      else // no se recibió trama del medidor al haberlo solicitado
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
            this->reInitComms();
            resp=1;
          }
          else // Retransmite último comando solicitado.
          {
            if(this->getCommCounter()==0)
            {
              if(this->isEncripted())
              {
                this->setDesencryptedMode();
              }
              else
              {
                this->setEncryptionMode();
              }
            }

            this->UART->myWDT->restartWdt();
            this->sendFrameToMPG2(this->commCounter);
          }
        }
      }

    }

  }
  return resp;
}
/***********************************************************/
char* CMPG2::getNMedidores()
{
  return &this->nMedidores;
}
/***********************************************************/
void CMPG2::setNMedidores(char nMed)
{
  this->nMedidores=nMed;
}
/***********************************************************/
void CMPG2::restartWRPointers()
{
  int x;
  for(x=0;x<this->nMedidores;x++)
  this->Flowmeter[x]->restartWRpointer(x);
}
/***********************************************************/
/*int CItzyFlow::strFloattoInt(char* m_Buffer)
{
   int tempDouble;
   int tempDoublefracc;
   int tempDoubleint;

 if(strcmp(m_Buffer,"nan")!=0)
 {
    if(m_Buffer[0]=='-')
    {
       sscanf(&m_Buffer[1],"%d.%3d",&tempDoubleint, &tempDoublefracc);
    }
    else
    {
       sscanf(&m_Buffer[0],"%d.%3d",&tempDoubleint, &tempDoublefracc);
    }
    if(tempDoublefracc<100)
    {
       tempDouble=(tempDoubleint*100)+tempDoublefracc;
    }
    else
    {
       tempDouble=(tempDoubleint*100)+(int)(tempDoublefracc/10);
    }
    if(m_Buffer[0]=='-')
    {
       tempDouble*=-1;
    }
 }
 else
 {
    tempDouble=0;
 }

 return tempDouble;
}
*/
/***********************************************************/
void CMPG2::reInitComms()
{
//  this->UART->statePort|=0x02;
  this->UART->Buffer_Tx[40]='\0';
  this->UART->statePort&=~0x02;
  this->UART->clearRxBuffPointer(this->UART); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
  this->UART->extraTimeCounter=0;
  this->UART->commTries=0;
  this->UART->strRxLen=0;
  this->UART->UartError&=~0x01;

}
/***********************************************************/
void CMPG2::set_ID_Rep(char ID)
{
  this->ID_Rep=ID;
}
/***********************************************************/
char CMPG2::get_ID_Rep()
{
  return this->ID_Rep;
}
/***********************************************************/
void CMPG2::setFlagTXExt()
{
  this->flagsTXExt=true;
}
/***********************************************************/
void CMPG2::clearFlagTXExt()
{
  this->flagsTXExt=false;
}
/***********************************************************/
bool CMPG2::getFlagTXExt()
{
  return this->flagsTXExt;
}
/***********************************************************/
void CMPG2::clearCommCounter()
{
  this->commCounter=0;
  this->configEncrytionPort();
}
/***********************************************************/
int CMPG2::getCommCounter()
{
  return this->commCounter;
}
/***********************************************************/
void CMPG2::incCommCounter()
{
  this->commCounter++;
}
/***********************************************************/
char CMPG2::getCRCbuffer(char* xBuffer, int length)
{
  length-=4;
  return (asctobcd(xBuffer[length]) * 0x10) | asctobcd(xBuffer[length+1]);
}
/***********************************************************/
char CMPG2::asctobcd(char asc)
{
  if (asc>=0x30 && asc<=0x39)
    asc-=0x30;
  else
    asc-=0x37;

  return asc&0x0F;
}
/***********************************************************/
bool CMPG2::evalSN(char* bufferRef,char* buffertoEval)
{
 int i;
 bool resp = false;
 for(i=0;i<10;i++)
 {
   if(bufferRef[i]!=buffertoEval[i])
   {
     resp=false;
     break;
   }
   else
   {
     resp=true;
   }
 }
  return resp;
}
/****************************************************************************/
void CMPG2::encryptData(long int *keyWord, char* eDataBuffer, int len, bool encrypt)
{
  int x,t;
  ldiv_t res;
  long int l_keyWord=*keyWord;
  int val;

  //if(this->isEncryptionMode())
  if(this->isEncripted())
  {
    // Encode eDataBuffer
    t=0;
    for(x=0;x<len-2;x++)
    {
      res=ldiv(l_keyWord,10);
      l_keyWord=res.quot;
      val=res.rem+1;
      t++;
      if(t>=5)
      {
        t=0;
        l_keyWord=*keyWord;
      }

      res=ldiv(x,2.0);
      if(res.rem)
      {
        if(encrypt)
        {
          eDataBuffer[x]=((eDataBuffer[x]+2)^0x0A)+val;
        }
        else
        {
          eDataBuffer[x]=((eDataBuffer[x]-val)^0x0A)-2;
        }
      }
      else
      {
        if(encrypt)
        {
          eDataBuffer[x]=((eDataBuffer[x]-3)^0x05)-val;
        }
        else
        {
          eDataBuffer[x]=((eDataBuffer[x]+val)^0x05)+3;
        }
      }
    }
  }
}
/**********************************************************************************/
void CMPG2::configEncrytionPort()
{
  P1DIR&=~portEncrypt;  // Configures port as input
  P1OUT&=~portEncrypt;   // Set out in low level
}
/**********************************************************************************/
bool CMPG2::isEncryptionMode()
{
  if(P1IN & portEncrypt)
  {
    return true;
  }
  return false;
}
/**********************************************************************************/
bool CMPG2::isEncripted()
{
  return this->flagEncrypted;
}
/**********************************************************************************/
void CMPG2::setEncryptionMode()
{
  this->flagEncrypted=true;
}
/**********************************************************************************/
void CMPG2::setDesencryptedMode()
{
  this->flagEncrypted=false;
}
/**********************************************************************************/
