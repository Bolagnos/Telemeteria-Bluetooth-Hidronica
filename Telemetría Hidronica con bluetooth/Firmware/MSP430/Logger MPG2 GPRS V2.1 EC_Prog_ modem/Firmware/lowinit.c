/*			- lowinit.c -

  The function __low_level_init is called by the start-up code before doing
  the normal initialization of data segments. If the return value is zero,
  initialization is not performed.

  In the run-time library there is a dummy __low_level_init, which does
  nothing but return 1. This means that the start-up routine proceeds with
  initialization of data segments.

  To replace this dummy, compile a customized version (like the example
  below) and link it with the rest of your code.

 */

/*
 * $Revision: 1.2 $
 */
#include "msp430x16x.h"
int __low_level_init(void)
{
  /* Insert your low-level initializations here */
    WDTCTL=WDTPW|((WDTCTL|WDTCNTCL|WDTHOLD)& 0x00FF);
    SVSCTL|=BIT7|PORON;
    P1DIR|=BIT7;  // Establece el puerto 1.7 como salida
    P1OUT|=BIT7;  // Salida en 1 (Enciende el modem GPRS)
  /*==================================*/
  /* Choose if segment initialization */
  /* should be done or not.		*/
  /* Return: 0 to omit seg_init	*/
  /*	       1 to run seg_init	*/
  /*==================================*/
  return (1);
}
