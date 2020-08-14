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
#include "msp430x16x.h"
#include "string.h"


char calcCRC(char* xBuffer, int length);

// HRDT;3001180002;04\n
// HRPA;3001180002;19568;0B\n
// HWUR;3001180002;0862;27\n

//---------------------------------
// HRPA;3001180001;19568;0A\n
// HWUR;3001180001;0862;26\n
//---------------------------------
// HRPA;3001180000;19568;09\n
// HWUR;3001180000;0862;25\n

const char* commandStr[] ={"HWIS;3001180000;M;",
                           "HWNE;3001180000;0812234;",
                           "HWNC;3001180000;HID950704EN7;",
                           "HWLC;3001180000;+19.365887;-99.185639;",
                           "HRMB;3001180000;000098E0;",
                           "HRMB;3001180000;000098E1;",
                           "HRMB;3001180000;000098E2;",
                           "HRMB;3001180000;000098E3;",
                           "HRMB;3001180000;000098E4;",
                           "HRMB;3001180000;000098E5;",
                           "HRMB;3001180000;000098E6;",
                           "HRMB;3001180000;000098E7;",
                           "HRMB;3001180000;000098E8;",
                           "HRMB;3001180000;000098E9;",
                           "HRMB;3001180000;000098EA;",
                           "HRMB;3001180000;000098EB;",
                           "HRMB;3001180000;000098EC;",
                           "HRMB;3001180000;000098ED;",
                           "HRMB;3001180000;000098EE;",
                           "HRMB;3001180000;000098EF;",
                           "HRMB;3001180000;000098F0;",
                           "HRMB;3001180000;000098F1;",
                           "HRMB;3001180000;000098F2;",
                           "HRMB;3001180000;000098F3;",
                           "HRMB;3001180000;000098F4;",
                           "HRMB;3001180000;000098F5;",
                           "HRMB;3001180000;000098F6;",
                           "HRMB;3001180000;000098F7;",
                           "HRMB;3001180000;000098F8;",
                           "HRMB;3001180000;000098F9;",
                          };

// HWIS;3001180000;M;79 Registro identificador del medidor o sistema de medicion
// HWNE;3001180000;0812234;1B No serie externo
// HWNC;3001180000;HID950704EN7;55  RFC
// HWLC;3001180000;+19.365887;-99.185639;13 localizazción GPS
// HRMB;3001180000;000098E0;52
//HRMB;3001180000;000098E1;51
//HRMB;3001180000;000098E2;54
//HRMB;3001180000;000098E3;53
//HRMB;3001180000;000098E4;56
//HRMB;3001180000;000098E5;55
//HRMB;3001180000;000098E6;58
//HRMB;3001180000;000098E7;57
//HRMB;3001180000;000098E8;5A
//HRMB;3001180000;000098E9;59
//HRMB;3001180000;000098EA;21
//HRMB;3001180000;000098EB;24
//HRMB;3001180000;000098EC;23
//HRMB;3001180000;000098ED;26
//HRMB;3001180000;000098EE;25
//HRMB;3001180000;000098EF;28
//HRMB;3001180000;000098F0;53
//HRMB;3001180000;000098F1;54
//HRMB;3001180000;000098F2;51
//HRMB;3001180000;000098F3;52
//HRMB;3001180000;000098F4;57
//HRMB;3001180000;000098F5;58
//HRMB;3001180000;000098F6;55
//HRMB;3001180000;000098F7;56
//HRMB;3001180000;000098F8;5B
//HRMB;3001180000;000098F9;5C


void main(void)
{
  int x;
  char CRC_Result[30];

  for(x=0;x<=29;x++)
  {
    CRC_Result[x]=calcCRC((char*)*&commandStr[x],strlen((char*)commandStr[x]));
  }

  CRC_Result[0]++;

   while(1)
   {
      LPM3;             // Low Power Mode 3
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
}
/**********************************************************************************/
/**********************************************************************************/
/*                       Comunicaciones UART0 por hardware                        */
/*                Rutina de interrupción Tx para módulo Bluetooth                 */
/**********************************************************************************/
#pragma vector=USART0TX_VECTOR
__interrupt void intUART0Tx(void)
{
  LPM3_EXIT;
}
/**********************************************************************************/
/*                       Comunicaciones UART0 por hardware                        */
/*                  Rutina de interrupción Rx para módulo GPRS                    */
/**********************************************************************************/
#pragma vector=USART0RX_VECTOR
__interrupt void intUART0Rx(void)
{
  LPM3_EXIT;
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
}
/**********************************************************************************/
/*                       Comunicaciones UART1 por hardware                        */
/*                     Rutina de interrupción Rx para Medidor                     */
/**********************************************************************************/
#pragma vector=USART1RX_VECTOR
__interrupt void intUART1Rx(void)
{
  LPM3_EXIT;
}
/**********************************************************************************/
/*                            Interrupción Timer B                                */
/*                     Rutina de generador de tiempo xxxxxxxx                     */
/**********************************************************************************/
#pragma vector=TIMERB0_VECTOR
__interrupt void intTimerB0(void)
{
  LPM3_EXIT;
}
/**********************************************************************************/
/*                            Interrupción Timer A                                */
/*             Rutina de generador de tiempo de 1 seg externo RTC                 */
/**********************************************************************************/
#pragma vector=TIMERA1_VECTOR
__interrupt void intTimerA1(void)
{
  LPM3_EXIT;
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
}
/**********************************************************************************/
/*                           Interrupción Puerto 1                                */
/*                            Rutina para teclado                                 */
/**********************************************************************************/
#pragma vector=PORT2_VECTOR
__interrupt void PORT2ISR (void)
{
 LPM3_EXIT;
}
/**********************************************************************************/
/*                              Interrupción DMA                                  */
/*                            Rutina para TX UARTS                                */
/**********************************************************************************/
#pragma vector=DACDMA_VECTOR
__interrupt void UARTS_O_1(void)
{
  LPM3_EXIT;
}
/**********************************************************************************/
#pragma vector = ADC12_VECTOR
__interrupt void intADC12(void)
{
  LPM3_EXIT;
}
/**********************************************************************************/

char calcCRC(char* xBuffer, int length)
{
  int CRC=0;
  for(int x=0; x<length; x++)
     CRC^=*xBuffer++;
  return CRC+1;
}
/**********************************************************************************/
