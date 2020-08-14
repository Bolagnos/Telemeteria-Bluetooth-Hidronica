#include "CDevice.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#define Num_ser_flash  0x1000  // Dirección del apuntador a número de serie del medidor en la región FLASH de Datos del microcontrolador.
//#define portEncrypt BIT2  // P1.2 Input to detect encryption function

//#define posBuffData 40
/****************************************************************************************************/
CMPG2::CMPG2()
{
  char x;

  //this->configEncrytionPort();

  for(x=0;x<NumMedidores;x++)
  {
    this->Flowmeter[x]=new CMedidor(x);
  }
  strcpy(this->serialNumber,"12020000");
  this->commCounter=0x00;
  this->nMedidores=NumMedidores; //0x01; //  1= 1 medidor, 2= 2 medidores...etc.
  this->ID_Rep=0x00;
  this->requestData=false;
  //this->flagsTXExt=false;
  this->flagEncrypted=true; // Bandera para encriptar=rue o no encriptar=false infromación
  UART=new CUart1();
}
/****************************************************************************************************/
CMPG2::~CMPG2()
{
}
/****************************************************************************************************/
const char* MPG2Command[]={"HRID:", // Solicitud No. Serie
                           //"HRPT:", // Tipo de producto
                           "HRFL:", // Flujo
                           "HRVL:", // Volumen
                           "HRSF:", // Estado de Flujo de Agua y batería
                           "HRUM:", // Unidades de medida
                          };
/****************************************************************************************************/
// Implementación de Funciones.
/****************************************************************************************************/
void CMPG2::sendFrameToMPG2()
{
  if(!this->commCounter)
  {
    this->UART->closeUart();
    this->UART->configDMAforFlowmeter();
    this->UART->openUart(this->UART,false);
  }

     int mylen=0;
     mylen=this->assembleStringMPG2(&this->commCounter);
     //mylen=this->assembleStringMPG2(&Command);
     this->UART->startTx((char *)this->UART->Buffer_Tx,mylen); // Envía la cadena de datos por el puerto serie.
}
/****************************************************************************************************/
/*void CMPG2::sendFrameTx(char* BufferTx, int lenght)
{
     int mylen=lenght;
     this->setEncryptionMode();
     this->encryptData((long int*)&keyWord, &BufferTx[0], lenght, true);  // Data encryption
     this->UART->startTx((char *)BufferTx,mylen); // Envía la cadena de datos por el puerto serie.
}*/
/****************************************************************************************************/
// El codigo evalúa sobre los separadores del formato de fecha y cantidad de dígitos del mes.
//int CMPG2::evalMedidorRxString(char* BufferRx,int RxLength,char* SerNo,CLogger* m_Logger,CRTC_3029_I2C *Reloj,CEEPROM *memory,char* dns,char* ipPort, char *apnName,char *apnUser, char *apnPsw, char *idSP)
int CMPG2::evalMedidorRxString(char* BufferRx,int RxLength,char* SerNo,CLogger* m_Logger,CRTC_3029_I2C *Reloj,CEEPROM *memory)
{
   int response=0x00;
   char m_CRC;

   if(this->requestData)
   {
    m_CRC=calcCRCMPG2(BufferRx,RxLength-4);
    if(m_CRC==this->getCRCbuffer(BufferRx, RxLength))
    {
      response=0x01;
    }
   }

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
//void CMPG2::parsingData_Units(char* DataBuffer, char* BufferStore, int* NCommand, int RxLength)
void CMPG2::parsingData_Units(char* DataBuffer, char* BufferStore, int* NCommand)
{
   int x=0;
   int i=posBuffData; //40;
   char *p_i,*p_j; //,*p_k;;
   char *startBuffer=DataBuffer; // Apunta al inicio de Buffer_Rx

switch(*NCommand)
{
case 0: // Obtiene Número de serie logico del medidor
        p_i=strstr(startBuffer,":");
        p_i++;
        p_j=strstr(startBuffer,";");
        x=p_j-p_i;
        strncpy(this->serialNumber,p_i,x);
        this->serialNumber[x]='\0';
        BufferStore[i]='\0';
        break;
/*case 1: // Obtiene ten número de serie lógico del medidor y lo estabece en la cadena de datos a almacenar en memoria.
        p_i=strchr(&BufferStore[i],'\0');
        p_i='\0';
        break;*/
case 1: // Obtiene Gasto Instantáneo
        p_j=strstr(startBuffer,":");
        p_j++;

        p_i=strchr(&BufferStore[i],'\0');
        *p_i++=*(p_j++); //startBuffer[x++];
        *p_i++=*(p_j++); //startBuffer[x++];

        switch(*(p_j+3))
        {
        case '4':
                  *p_i++=*(p_j++); //startBuffer[x++];
                  *p_i++=*(p_j++); //startBuffer[x++];
                  break;
        case '3':
                  *p_i++=*(p_j++); //startBuffer[x++];
                  *p_i++='.';
                  *p_i++=*(p_j++); //startBuffer[x++];
                  break;
        case '2':
                  *p_i++='.';
                  *p_i++=*(p_j++); //startBuffer[x++];
                  *p_i++=*(p_j++); //startBuffer[x++];
        default:  break;
        }
        *p_i++=',';
        *p_i++='\0';
        break;
case 2: // Obtiene Volumen Acumulado
        //x=5;

        p_j=strstr(startBuffer,":");
        p_j++;
        //p_k=strstr(startBuffer,";");

        //x=p_j-p_k;

        p_i=strchr(&BufferStore[i],'\0');
        //strncpy(p_i,&startBuffer[5],12); // Dividorlo en servidor por 100 para ponerlo como litros, dividirlo por 100,000 para m^3
        //strncpy(p_i,&startBuffer[x],7);
        strncpy(p_i,p_j,7);

        p_i+=7;
        p_j+=7;

        *p_i++='.';
        //*p_i='\0';
        strncpy(p_i,p_j,5);
        p_i+=5;
        *p_i++=',';
        *p_i++='\0';
        break;
case 3:  // Obtiene estado de la batería
        //x=7;
        p_j=strstr(startBuffer,":");
        p_j+=3;
        p_i=strchr(&BufferStore[i],'\0');
        *p_i++=*(p_j++); //startBuffer[x++];
        *p_i++=*(p_j++); //startBuffer[x++];
        *p_i++='\0';
        break;
default:  // Codigo para identificación del tipo de unidades
        break;
}

   for(x=0;x<this->UART->strRxLen;x++)
       DataBuffer[x]=0x00;


}
/****************************************************************************************************/
// resp=2 indica bandera para habilitar transmisiones via Satelital. (Finaliza secuencia de comandos requeridos).
//resp=1 indica finalización cualquier transmisión.
//resp=0 indica retransmiciones o indicación de tx del siguiente comando.

//int CMPG2::timeoutTxRx(char* SerNo,CLogger* m_Logger,CRTC_3029_I2C *Reloj,CEEPROM *memory,char* dns,char* ipPort, char *apnName,char *apnUser, char *apnPsw, char *idSP)
int CMPG2::timeoutTxRx(char* SerNo,CLogger* m_Logger,CRTC_3029_I2C *Reloj,CEEPROM *memory)
{
  int resp=0;

  if(this->UART->statePort&0x02)  // Comunicaciones Tx o Rx del medidor Milltronics?
  {
    this->UART->myWDT->stopWdt();
    this->UART->statePort&=~0x02;

    if (this->UART->UartError==0x01)  // Verifica si existe Error de Over flow del buffer de Rx.
    {
      this->UART->clearRxBuffPointer(); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
      this->UART->UartError&=~0x01;
    }
    else
    {
      if(this->UART->strRxLen)// Se recibió una cadena de datos.
      {
        this->UART->Buffer_Rx[this->UART->strRxLen]=0x00; // Almacena en el ultimo byte de la cadena de datos recibidos el caracter 0.
        this->encryptData((long int*)&keyWord, (char*)&this->UART->Buffer_Rx[0], this->UART->strRxLen, false); // Data decryption
        //resp=this->evalMedidorRxString((char*)&this->UART->Buffer_Rx[0],this->UART->strRxLen,SerNo,m_Logger,Reloj,memory,dns,ipPort,apnName,apnUser,apnPsw,idSP);
        resp=this->evalMedidorRxString((char*)&this->UART->Buffer_Rx[0],this->UART->strRxLen,SerNo,m_Logger,Reloj,memory);
        if (resp==0x01) // Se recibió la respuesta de manera correcta del medidor?
        {
          //this->parsingData_Units((char*)&this->UART->Buffer_Rx[0], (char*)&this->UART->Buffer_Tx[0], &this->commCounter,this->UART->strRxLen);
          this->parsingData_Units((char*)&this->UART->Buffer_Rx[0], (char*)&this->UART->Buffer_Tx[0], &this->commCounter);

          this->incCommCounter();
          this->UART->extraTimeCounter=0;
          this->UART->commTries=0;
          this->UART->clearRxBuffPointer(); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
          this->UART->strRxLen=0;
          this->UART->UartError&=~0x01;

          if(this->getCommCounter()>3)
          {// Finaliza las comunicaciones una vez que se obtuvo la información del medidor
            this->clearCommCounter();
            resp=2;
          }
          else // De otro modo se envía el siguiente comando para el medidor MPG2
          {
            this->UART->myWDT->restartWdt();
            //this->sendFrameToMPG2(this->commCounter);
            this->sendFrameToMPG2();
            resp=0;
          }

        }
        else  //No se recibió la respuesta correcta o falta información.
        {
          /*if(resp)
          {
            this->setFlagTXExt();
            this->sendFrameTx((char*)&this->UART->Buffer_Rx[0], resp);
          }*/
          /*else
          {*/
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
                  this->UART->clearRxBuffPointer(); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
                  this->UART->extraTimeCounter=0;
                  this->UART->commTries++;
                  if(this->UART->commTries>this->UART->tries) // Cantidad de intentos de retransmisión en caso de que no haya recibido ningún byte en respuesta.
                  {
                    //Finaliza retransimisiones a Medidor por haber esperado por más información
                    this->reInitComms();
                    resp=3;
                  }
                  else // Retransmite último comando solicitado.
                  {
                    this->UART->myWDT->restartWdt();
                    //this->sendFrameToMPG2(this->commCounter);
                    this->sendFrameToMPG2();
                  }
                }
             }
             else // Se recibió cadena sin algun significado
             {
               this->reInitComms();
               resp=1;
             }
          //}
/*          if(this->UART->extraTimeCounter<this->UART->extraTimeout)// Espera de tiempo extra en espera para recibir datos en caso que no se haya recibido ningún byte.
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
            }
            else // Retransmite último comando solicitado.
            {
               this->sendFrameToPanametrics(this->commCounter);
            }
          }
          */
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
          this->UART->clearRxBuffPointer(); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
          this->UART->extraTimeCounter=0;
          this->UART->commTries++;
          if(this->UART->commTries>this->UART->tries) // Cantidad de intentos de retransmisión en caso de que no haya recibido ningún byte en respuesta.
          {
            //Finaliza retransimisiones a Medidor si no recibió información
            this->reInitComms();
            resp=3;
          }
          else // Retransmite último comando solicitado.
          {
            if(this->getCommCounter()==0)
            {
              if(this->isEncrypted())
              {
                this->setDesencryptedMode();
              }
              else
              {
                this->setEncryptionMode();
              }
            }

            this->UART->myWDT->restartWdt();
            //this->sendFrameToMPG2(this->commCounter);
            this->sendFrameToMPG2();
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
void CMPG2::restartWRPointers(int offset)
{
  int x;
  for(x=0;x<this->nMedidores;x++)
  {
    if (!offset)
    {
      this->Flowmeter[x]->restartWRpointer(x);
    }
    else
    {
      this->Flowmeter[x]->restartWRpointerOffset(x, offset);

    }
  }
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
  //this->UART->Buffer_Tx[40]='\0';
  this->UART->Buffer_Tx[posBuffData]='\0';
  this->UART->statePort&=~0x02;
  this->UART->clearRxBuffPointer(); // Inicializa puntero de Rx en el Buffer de Rx de Comunicaciones Serie
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
/*void CMPG2::setFlagTXExt()
{
  this->flagsTXExt=true;
}*/
/***********************************************************/
/*void CMPG2::clearFlagTXExt()
{
  this->flagsTXExt=false;
}*/
/***********************************************************/
/*bool CMPG2::getFlagTXExt()
{
  return this->flagsTXExt;
}*/
/***********************************************************/
void CMPG2::clearCommCounter()
{
  this->commCounter=0;
//  this->configEncrytionPort();
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
/*bool CMPG2::evalSN(char* bufferRef,char* buffertoEval)
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
}*/
/****************************************************************************/
void CMPG2::encryptData(long int *keyWord, char* eDataBuffer, int len, bool encrypt)
{
  int x,t;
  ldiv_t res;
  long int l_keyWord=*keyWord;
  int val;

  //if(this->isEncryptionMode())
  if(this->isEncrypted())
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
/*void CMPG2::configEncrytionPort()
{
  P1DIR&=~portEncrypt;  // Configures port as input
  P1OUT&=~portEncrypt;   // Set out in low level
}*/
/**********************************************************************************/
/*bool CMPG2::isEncryptionMode()
{
  if(P1IN & portEncrypt)
  {
    return true;
  }
  return false;
}*/
/**********************************************************************************/
bool CMPG2::isEncrypted()
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
char* CMPG2::getSerialNumberFM()
{
  return &this->serialNumber[0];
}
/**********************************************************************************/
void CMPG2::setFailRegisters(char* Buffer)
{
  int i=posBuffData; //40;
  int x;

  for(x=0;x<20;x++)
  {
    switch(x)
    {
    //case 21: Buffer[i++]='\0'; break;
    //case 20: Buffer[i++]='U'; break;
    //case 19: Buffer[i++]='B'; break;
    case 19:
    case 5:  Buffer[i++]=','; break;
    case 13:
    case 2:  Buffer[i++]='.'; break;
    default: Buffer[i++]='0'; break;
    }
  }
  Buffer[i++]='B';
  Buffer[i++]='U';
  Buffer[i++]='\0';
/*
  Buffer[i++]='0';
  Buffer[i++]='0';
  Buffer[i++]='.';
  Buffer[i++]='0';
  Buffer[i++]='0';
  Buffer[i++]=',';
  Buffer[i++]='0';
  Buffer[i++]='0';
  Buffer[i++]='0';
  Buffer[i++]='0';
  Buffer[i++]='0';
  Buffer[i++]='0';
  Buffer[i++]='0';
  Buffer[i++]='.';
  Buffer[i++]='0';
  Buffer[i++]='0';
  Buffer[i++]='0';
  Buffer[i++]='0';
  Buffer[i++]='0';
  Buffer[i++]=',';
  Buffer[i++]='B';
  Buffer[i++]='U';
  Buffer[i++]='\0';
*/

}
/**********************************************************************************/

