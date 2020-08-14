
#include "msp430x16x.h"

/*
#ifndef TIMER0_A1_VECTOR
#define TIMER0_A1_VECTOR TIMERA1_VECTOR
#define TIMER0_A0_VECTOR TIMERA0_VECTOR
#endif
*/
void flash_write_segmentD (unsigned char *value, unsigned char position);
void flash_read_segmentD (unsigned char *var, unsigned char position);

#define NBytes 6

void main (void)
{
  unsigned char m_SerialNumber[NBytes]={'1','8','0','0','0','2'};

  flash_write_segmentD ((unsigned char*)&m_SerialNumber[0], 0x00);

  for(;;)
  {}
}



void flash_write_segmentD (unsigned char *value, unsigned char position)
{
char *Flash_ptr; // Flash pointer
int x=0;

Flash_ptr = (char *) 0x1000; // Initialize Flash pointer
Flash_ptr += position; // Jump to desired address
FCTL1 = FWKEY + ERASE; // Set Erase bit
FCTL3 = FWKEY; // Clear Lock bit
*Flash_ptr = 0; // Dummy write to erase Flash segment

FCTL1 = FWKEY + WRT; // Set WRT bit for write operation

for(x=0;x<NBytes;x++)
{
  *Flash_ptr = *value++; // Write value to flash
  Flash_ptr++;
}

FCTL1 = FWKEY; // Clear WRT bit
FCTL3 = FWKEY + LOCK; // Set LOCK bit
}

void flash_read_segmentD (unsigned char *var, unsigned char position)
{
char *Flash_ptr; // Segment C pointer

Flash_ptr = (char *) 0x1000; // Initialize Flash segment C pointer
Flash_ptr += position; // Jump to desired address

*var = *Flash_ptr; // copy value segment C to variable

}

/////////////////////////////////////////////////////////////////////////////


