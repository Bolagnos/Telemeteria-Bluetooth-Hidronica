#include "CDevice.h"
//#include "string.h"
#include "string"

#define Num_ser_flash  0x1000  // Dirección del apuntador a número de serie del medidor en la región FLASH de Datos del microcontrolador.
/****************************************************************************************************/

/****************************************************************************************************/
CBascula::CBascula()
{
  this->commCounter=0x00;
  this->requestData=false;
  UART=new CUart1();
}
/****************************************************************************************************/
CBascula::~CBascula()
{
}
/****************************************************************************************************/
const unsigned char *BasculaCommand[]={//"680%s23640%e5%e%e682.3%s#IDB%e684.1%s2%e684.2%s2%e684.3%s2%e684.4%s2%e684.5%s2%e686.1%s1%e686.2%s1%e686.3%s3%e%s11%e686.4%s1%e686.5%s1%e989%s1%e%e%s%s1%e%s1%e%s0%e992%s1%e1000%s%u%c%eHMDT: %e%e0.0%e32768%e%e.256%e%e11%e34835%e%e.256%e%e80.3%e32768%e%e.256%e.000%e503%s0%e%s1%e%z%c%e%e3%i0701070001%e%s", // No serie XXVVYYNNNN  XX=# producto, VV=Ver Firm: YY = año de elab; NNNN= # consecutivo
                                       "680%s23640%e5%e%e682.3%s#IDB%e684.1%s2%e684.2%s2%e684.3%s2%e684.4%s2%e684.5%s2%e686.1%s1%e686.2%s1%e686.3%s3%e%s11%e686.4%s1%e686.5%s1%e989%s1%e%e%s%s1%e%s1%e%s0%e992%s1%e1000%s%u%c%eHMDT: %e%e0.0%e32768%e%e.256%e%e11%e34835%e%e.256%e%e80.3%e32768%e%e.256%e.000%e503%s0%e%s1%e%z%c%e%e3%i", // No serie XXVVYYNNNN  XX=# producto, VV=Ver Firm: YY = año de elab; NNNN= # consecutivo
                                       "%e%s",
                                       "9990.1%s23640%e%c%e1%e%e19999.1%s%c%e1%%@Conf Hrs?%%Y%%{1%%T# Tx 0-3:%%n%%\\1%%J%%N%%{=80.4P%%o80.4P>3%%o1%%J%%N%%{80.4P<0%%o1%%J%%N%%{%eHMTN: %%$80.4P<10%%o0%%$%%N%%E80.4P%%o%%$13,10%%&%%}%%E%%}%%E%%}%%E80.5P=0%%o4%%T80.5P<80.4P%%o%%%e{80.5P+=1%%o80.5P%%o%%[2%%TTx Hr %%]:%%Phr%%]  0-23:%%n%%\\2%%J%%N%%{=80.1P%%o80.1P>23%%o2%%J%%N%e%%{80.1P<0%%o2%%J%%N%%E%%}%%E%%}%%E3%%Tmin%%] 0-59:%%n%%\\3%%J%%N%%{=80.2P%%o80.2P>59%%o3%%J%%N%%{80.%e2P<0%%o3%%J%%N%%E%%}%%E%%}%%EConf Hr%%]?%%Y%%{HMSA: %%$80.5P<10%%o0%%$%%N%%E80.5P%%o %%$80.1P<10%%%eo0%%$%%N%%E80.1P%%o:%%$80.2P<10%%o0%%$%%N%%E80.2P%%o%%$13,10%%&11.0.34835P%%o%%$13,10%%&%%}%%N%%E%e4%%J%%}%%N%%E%%}%%N%%E%e%z%c%e%e",
                                       "1%s0%u%p",
                                      };
/****************************************************************************************************/
// Implementación de Funciones.
/****************************************************************************************************/
void CBascula::sendFrameToBascula(CUart1 * uartPort, int Command, CBascula* myBascula)
{
     int mylen=0;

     //sprintf((char *)&uartPort->Buffer_Tx[0],"%s",BasculaCommand[Command]);
     //mylen=strlen((char *)&uartPort->Buffer_Tx[0]);

     //uartPort->startTx(uartPort,(char *)&uartPort->Buffer_Tx[0],mylen); // Envía la cadena de datos por el puerto serie.

     mylen=strlen((char *)&BasculaCommand[Command][0]);

     if(Command==1) //Establece el numero de serie grabado sobre la linea de configuración de la báscula GSE 465
     {
        //sprintf((char *)&uartPort->Buffer_Tx[0],"%s",GPRSCommand[Command]);
        strncpy((char *)uartPort->Buffer_Tx,(char*) Num_ser_flash ,10); //12
        uartPort->Buffer_Tx[10]='\0';
        strncat((char *)uartPort->Buffer_Tx,(char*)BasculaCommand[1],strlen((char*)BasculaCommand[Command]));
        mylen=strlen((char*)uartPort->Buffer_Tx);

        uartPort->startTx(uartPort,(char *)uartPort->Buffer_Tx,mylen); // Envía la cadena de datos por el puerto serie.
     }
     else
     {
        uartPort->startTx(uartPort,(char *)&BasculaCommand[Command][0],mylen); // Envía la cadena de datos por el puerto serie.
     }

}
/****************************************************************************************************/
/****************************************************************************************************/
unsigned char CBascula::evalBasculaResponse(unsigned char* BufferRx, CRTC_3029_I2C *myClock)
{
  unsigned char response=0x00;
  char * mypointer;
  char  Buffer[4];
  //char  nAlarm;

  switch(BufferRx[0])
  {

  case 'H':switch(BufferRx[1])
          {
          case 'M':switch(BufferRx[2])
                   {
                   case 'D':switch(BufferRx[3])
                            {
                            case 'T':             // Implementar rutina para establecer el buffer de Rx a transimitir vía módulo de GSM
                                     P5OUT^=0x04;
                                     mypointer=strstr((char *)&BufferRx[0],"Tm/Dt");
                                     sscanf(mypointer+6, "%2d", (int*)&Buffer[0]);
                                     myClock->hour=Buffer[0];
                                     sscanf(mypointer+9, "%2d", (int*)&Buffer[0]);
                                     myClock->min=Buffer[0];
                                     sscanf(mypointer+12, "%2d", (int*)&Buffer[0]);
                                     myClock->seg=Buffer[0];
                                     sscanf(mypointer+15, "%2d", (int*)&Buffer[0]);
                                     myClock->day=Buffer[0];
                                     sscanf(mypointer+18, "%2d", (int*)&Buffer[0]);
                                     myClock->month=Buffer[0];
                                     sscanf(mypointer+21, "%2d", (int*)&Buffer[0]);
                                     myClock->year=Buffer[0];
                                     response=0x01;
                                     break;
                            default:
                                    break;
                            }
                           break;
                   case 'S':switch(BufferRx[3])
                            {
                            case 'A':             // Implementer rutina para cambiar la fecha y hora de la interfáz y registro de alarma (Hora de Tx vía módulo de GSM).
                                     P5OUT^=0x08;
                                     sscanf((char*)&BufferRx[6], "%2d", (int*)&Buffer[0]);
                                     //nAlarm=Buffer[0]-1;
                                     sscanf((char*)&BufferRx[9], "%2d", (int*)&Buffer[0]);
                                     sscanf((char*)&BufferRx[12], "%2d", (int*)&Buffer[2]);
                                     //myClock->alarms[nAlarm]->setAlarm(&Buffer[0],&Buffer[2]);

                                     mypointer=strstr((char *)&BufferRx[0],"Tm/Dt");
                                     sscanf(mypointer+6, "%2d", (int*)&Buffer[0]);
                                     myClock->hour=Buffer[0];
                                     sscanf(mypointer+9, "%2d", (int*)&Buffer[0]);
                                     myClock->min=Buffer[0];
                                     sscanf(mypointer+12, "%2d", (int*)&Buffer[0]);
                                     myClock->seg=Buffer[0];
                                     sscanf(mypointer+15, "%2d", (int*)&Buffer[0]);
                                     myClock->day=Buffer[0];
                                     sscanf(mypointer+18, "%2d", (int*)&Buffer[0]);
                                     myClock->month=Buffer[0];
                                     sscanf(mypointer+21, "%2d", (int*)&Buffer[0]);
                                     myClock->year=Buffer[0];
                                     response=0x01;
                                     break;
                            default:
                                    break;
                            }
                           break;
                   case 'T':switch(BufferRx[3])
                            {
                            case 'N':             // Implementar rutina para establecer el buffer de Rx a transimitir vía módulo de GSM
                                     P5OUT^=0x10;
                                     sscanf((char*)&BufferRx[6], "%2d", (int*)&Buffer[0]);
//                                     myClock->nAlarms=Buffer[0];
                                     response=0x01;
                                     break;
                            default:
                                    break;
                            }
                           break;
                   default:
                           break;
                   }
                   break;
          default:
                  break;
          }
          break;

  default:
          break;
  }


  return response;
}
/****************************************************************************************************/

