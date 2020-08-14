#include "CController.h"

/*******************************************************************************************/
CClockModule::CClockModule()
{}
/*******************************************************************************************/
CClockModule::~CClockModule()
{}
/*******************************************************************************************/
void CClockModule::configSMCLK(char clk_source, char div_clk)
{
  switch(clk_source)
  {
  case 'D':   // Chosse DCOCLK
            BCSCTL2&=~SELS;
            BCSCTL1&=~XT2OFF;
            break;
  case 'O':   // Choose External OSC (XT2)
            BCSCTL2|=SELS;
            BCSCTL1|=XT2OFF;
            break;
  }

  BCSCTL2&=~DIVS_3;
  switch(div_clk)
  {
  case '2': BCSCTL2|=DIVS_1; break;
  case '4': BCSCTL2|=DIVS_2; break;
  case '8': BCSCTL2|=DIVS_3; break;
  }


}
/*******************************************************************************************/

