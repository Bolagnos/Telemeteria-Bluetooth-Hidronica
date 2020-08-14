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

char calcCRC(char* xBuffer, int length);

const char* commandStr[] ={"HRDT;3001150002;"};

int main(void)
{
  calcCRC((char*)*commandStr,strlen((char*)commandStr));


   while(true)
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