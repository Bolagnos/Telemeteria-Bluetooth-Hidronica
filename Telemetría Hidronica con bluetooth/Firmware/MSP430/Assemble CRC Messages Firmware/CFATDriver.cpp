#include "CDevice.h"
#include "stdlib.h"
#include "string.h"

const char* table_Months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
const char* txtExtension="txt";

CFATDriver::CFATDriver(CUart1* l_Uart)
{
  this->SDMem=new CSDMemoryCard(l_Uart);
}
/****************************************************************************************************/
CFATDriver::~CFATDriver()
{}
/****************************************************************************************************/
void CFATDriver::readBootBlock()
{
  int x;
  char* l_pChar;

  this->typeCode=0;
  this->LBA_begin=0;
  this->No_Sectors_Mem=0;

  char* p_data=this->SDMem->startReadFromMem(0x00000000);
  if(p_data && p_data[510]==0x55 && p_data[511]==0xAA)
  {
   this->typeCode=*(p_data+450);  // Obtiene el tipo de código, debe de ser 0x0B o 0xOC
   if(this->typeCode==0x0B || this->typeCode==0x0C) //Valida que sea un formato FAT 32
   {
    l_pChar=(char*)&this->LBA_begin;
    p_data+=454;
    for(x=0;x<4;x++)
    {
        l_pChar[x]=p_data[x];
    }

    l_pChar=(char*)&this->No_Sectors_Mem;
    p_data+=4;
    for(x=0;x<4;x++)
    {
        l_pChar[x]=p_data[x];
    }
   }
   else
   {
     typeCode=0;
   }
  }

}
/****************************************************************************************************/
void CFATDriver::readSystemPartition()
{
  int x;
  char* l_pChar;

  char* p_data=this->SDMem->startReadFromMem(this->LBA_begin);

  this->bytes_per_sector=0;
  this->sector_per_cluster=0;
  this->reserved_sectors=0;
  this->fats_No=0;
  this->sectors_per_fat=0;
  this->root_directory_first_cluster=0;
  this->cluster_begin_lba=0;

  if(p_data && p_data[510]==0x55 && p_data[511]==0xAA)
  {
    l_pChar=(char*)&this->bytes_per_sector;
    p_data+=11;
    for(x=0;x<2;x++)
    {
        l_pChar[x]=p_data[x];
    }

    p_data+=2;
    this->sector_per_cluster=*p_data;

    l_pChar=(char*)&this->reserved_sectors;
    p_data++;
    for(x=0;x<2;x++)
    {
        l_pChar[x]=p_data[x];
    }

    p_data+=2;
    this->fats_No=*p_data;

    l_pChar=(char*)&this->sectors_per_fat;
    p_data+=20;
    for(x=0;x<4;x++)
    {
        l_pChar[x]=p_data[x];
    }

    l_pChar=(char*)&this->root_directory_first_cluster;
    p_data+=8;
    for(x=0;x<4;x++)
    {
        l_pChar[x]=p_data[x];
    }
     this->cluster_begin_lba=this->LBA_begin+this->reserved_sectors+(this->fats_No*this->sectors_per_fat);
  }
}
/****************************************************************************************************/
void CFATDriver::readFAT(unsigned long int n_Block)
{
  this->SDMem->startReadFromMem(n_Block);
}
/****************************************************************************************************/
bool CFATDriver::writeFAT(unsigned long int n_Block)
{
  return this->SDMem->startWriteOnMem(n_Block);
}
/****************************************************************************************************/
/*Returns initial physical addres form Rx buffer where data starts.*/
char* CFATDriver::findSpaceOnMemFolder(unsigned long int clusterNo)
{
  int k;
  int l_pChar;
  char *p_data;
  char *result=0x0000;

  //this->l_FATsector=this->cluster_begin_lba;
  this->l_FATsector=clusterNo;
  for(k=0;k<8;k++)    // 8 represents the qty of blocks (1 cluster) for year folders using 32 bytes per register (128 years)
  {
    l_pChar=0x00;
    this->l_block_sector=this->cluster_begin_lba+k;
    p_data=this->SDMem->startReadFromMem(this->cluster_begin_lba+k);

    while(l_pChar<blockSize)
    {
      if(p_data[l_pChar]==0xE5 ||  p_data[l_pChar]==0x00)
      {
        break;
      }
      l_pChar+=32;
    }
    if(l_pChar>=0 && l_pChar < blockSize)
    {
      this->l_pos_addr_sector=l_pChar;

      if(this->l_block_sector)
      {
        this->l_pos_addr_sector+=(this->l_block_sector-this->cluster_begin_lba)*blockSize;
      }

      result=p_data;
      break;
    }
  }

  return result; // returns initial physical addres form Rx buffer where data starts.
}
/****************************************************************************************************/
void CFATDriver::clearBufferSDTx()
{
  this->SDMem->clearBufferSDTx();
}
/****************************************************************************************************/
/*For using "this->l_block_sector" and "this->l_pos_addr_sector" must be known using findSpaceOnMemFolder() function*/
//void CFATDriver::createsNewYearFolder(char* p_data, char* folderName, CLogger *p_loggerMem, CEEPROM *p_I2CMem)
void CFATDriver::createsNewYearFolder(char* p_data, char* year, CLogger *p_loggerMem, CEEPROM *p_I2CMem)
{
  int x,k; //,tries;
  int addr_offset=this->l_pos_addr_sector&0x01FF;
  unsigned long int calc_cluster,calc_clusterII,calc_cluster_before;
//  unsigned long int clusters_array[12];
  char* pl_data;
  char* pl_data2;

  //char flagEmpty=0x00;
  /*DMA0CTL&=~DMAIE;
  DMA1CTL&=~DMAIE;
  DMA2CTL&=~DMAIE;*/

  if(p_data)
  {

    for(x=0;x<=512;x++)    // Copy Rx buffer to Tx buffer
    {
      this->SDMem->UART->SD_Buffer_Tx[x]=p_data[x];
    }

    /*if(this->SDMem->UART->SD_Buffer_Tx[addr_offset]==0x00)
    {
      flagEmpty=0x01;
    }*/
    // Inserts year folder name
    for(x=0;x<4;x++)
    {
      this->SDMem->UART->SD_Buffer_Tx[addr_offset+x]=year[x];
    }
    x=11;
    this->SDMem->UART->SD_Buffer_Tx[addr_offset+x]=0x10; // Inserts ID Attr

    // Inserts MSB and LSB cluster sector

    //  calc_1=(((this->l_pos_addr_sector/32)*(this->sector_per_cluster*clustersPerMonth))+1);
    //    calc_1=(((this->l_pos_addr_sector/32)*(this->sector_per_cluster*(24)))+1); // Calulates size of cluters in a year for month registers folder
    calc_cluster=((this->l_pos_addr_sector/32)+1); // Calulates size of cluters in a year for month registers folder


    x=21;
    pl_data=(char*)&calc_cluster+3;

    for(;x>19;x--)
    {
      this->SDMem->UART->SD_Buffer_Tx[addr_offset+x]=*pl_data;
      pl_data--;
    }

    x=27;
    for(;x>25;x--)
    {
      this->SDMem->UART->SD_Buffer_Tx[addr_offset+x]=*pl_data;
      pl_data--;
    }


    // Stores 512 bytes info. on I2C EEPROM Memory 0x60600
    for(x=0;x<4;x++)
    {
      p_loggerMem->loggingData(addrBackupSDMem+(128*x), (char*)&this->SDMem->UART->SD_Buffer_Tx[128*x], 128, p_I2CMem); // Solo para una memoria de 128 KB
    }

    //p_I2CMem->setFlagBckUpSD();

    p_I2CMem->enableFlagWritingBckUpSD();
    /*DMA0CTL|=DMAIE;
    DMA1CTL|=DMAIE;
    DMA2CTL|=DMAIE;*/
    if(this->SDMem->startInitializeMem())
    {
    this->writeFAT(this->l_block_sector); // Writes on root folder memory space

    /*DMA0CTL&=~DMAIE;
    DMA1CTL&=~DMAIE;
    DMA2CTL&=~DMAIE;*/

    calc_cluster_before=this->l_block_sector-this->l_block_sector;

    /*if(flagEmpty)
    {
      flagEmpty=0x00;*/

      // Crear subcarpetas desde Enero a Diciembre respecto al año creado
      //IE2&=~(URXIE1);
      //IFG2&=~(UTXIFG1|URXIFG1);
      this->clearBufferSDTx();

      calc_cluster=(this->l_pos_addr_sector/32)+1;
      // Inserts Month folders name
      for(k=0;k<12;k++)
      {
        for(x=0;x<3;x++)
        {
          this->SDMem->UART->SD_Buffer_Tx[x+(k*32)]=(unsigned char)table_Months[k][x];
        }
        x=11;
        this->SDMem->UART->SD_Buffer_Tx[x+(k*32)]=0x10;

        calc_cluster=(this->l_pos_addr_sector/32)+1;  // Inserts local cluster number

        pl_data=(char*)&calc_cluster;
        for(x=12;x<16;x++)
        {
          this->SDMem->UART->SD_Buffer_Tx[x+(k*32)]=*pl_data;
          pl_data++;
        }

        pl_data=(char*)&calc_cluster_before;
        for(x=16;x<20;x++)
        {
          this->SDMem->UART->SD_Buffer_Tx[x+(k*32)]=*pl_data;
          pl_data++;
        }

        //calc_cluster=((this->l_pos_addr_sector/32)*12)+((this->bytes_per_sector/32)*this->sector_per_cluster)+k+1; //
        calc_cluster=((this->l_pos_addr_sector/32)*12)+((this->bytes_per_sector/32)*this->sector_per_cluster)+k+1; //

        x=21;
        pl_data=(char*)&calc_cluster+3;

        for(;x>19;x--)
        {
          this->SDMem->UART->SD_Buffer_Tx[x+(k*32)]=*pl_data;
          pl_data--;
        }

        x=27;
        for(;x>25;x--)
        {
          this->SDMem->UART->SD_Buffer_Tx[x+(k*32)]=*pl_data;
          pl_data--;
        }
      }

      for(x=0;x<4;x++) // escribe en I2C EEPROM 4 bloques de 128 Bytes competando 512 bytes
      {
        p_loggerMem->loggingData(addrBackupSDMem+(128*x), (char*)&this->SDMem->UART->SD_Buffer_Tx[128*x], 128, p_I2CMem); // Solo para una memoria de 128 KB
      }

      /*DMA0CTL|=DMAIE;
      DMA1CTL|=DMAIE;
      DMA2CTL|=DMAIE;*/
      p_I2CMem->enableFlagWritingBckUpSD();
      if(this->SDMem->startInitializeMem())
      {
      this->writeFAT((((this->l_pos_addr_sector/32)+1)*this->sector_per_cluster)+cluster_begin_lba); // Wirtes on cluster for each year folder

      /*DMA0CTL&=~DMAIE;
      DMA1CTL&=~DMAIE;
      DMA2CTL&=~DMAIE;*/

      //(this->l_block_sector-this->cluster_begin_lba)*0x0100)|

      //////Reads year folder by months cluster and gets all cluster adresses per month


      /*DMA0CTL|=DMAIE;
      DMA1CTL|=DMAIE;
      DMA2CTL|=DMAIE;*/
      if(this->SDMem->startInitializeMem())
      {
        /*DMA0CTL&=~DMAIE;
        DMA1CTL&=~DMAIE;
        DMA2CTL&=~DMAIE;*/

        calc_clusterII=(((this->l_pos_addr_sector/32)+1)*this->sector_per_cluster)+cluster_begin_lba;
      //this->SDMem->startInitializeMem();
        /*DMA0CTL|=DMAIE;
        DMA1CTL|=DMAIE;
        DMA2CTL|=DMAIE;*/

        pl_data=this->SDMem->startReadFromMem(calc_clusterII);

        /*DMA0CTL&=~DMAIE;
        DMA1CTL&=~DMAIE;
        DMA2CTL&=~DMAIE;*/

      //pl_data=this->SDMem->startReadFromMem((((this->l_pos_addr_sector/32)+1)*this->sector_per_cluster)+cluster_begin_lba);

      //creates file into each folder by month
      //calc_clusterII=;

        for(k=0;k<12;k++)
        {
          this->clearBufferSDTx();

          x=21;
          calc_cluster=pl_data[x]*0x01000000 | pl_data[x-1]*0x00010000;
          x+=6;
          calc_cluster|=pl_data[x]*0x00000100 | pl_data[x-1]*0x00000001;

          x=12;
          calc_cluster_before=pl_data[x]*0x00000001 | pl_data[x+1]*0x00000100 | pl_data[x+2]*0x00010000 | pl_data[x+3]*0x01000000;

          for(x=0;x<11;x++)
          {
            if(x<3)
            {
              this->SDMem->UART->SD_Buffer_Tx[x]=(unsigned char)table_Months[k][x];
            }
            else if(x>=8)
            {
              this->SDMem->UART->SD_Buffer_Tx[x]=(unsigned char)txtExtension[x-8];
            }
            else
            {
              this->SDMem->UART->SD_Buffer_Tx[x]=0x20;
            }
          }

          this->SDMem->UART->SD_Buffer_Tx[x]=0x20; // ID Attrib File

          pl_data2=(char*)&calc_cluster;
          for(x=12;x<16;x++)
          {
            this->SDMem->UART->SD_Buffer_Tx[x]=pl_data2[x-12];
          }

          pl_data2=(char*)&calc_cluster_before;
          for(x=16;x<20;x++)
          {
            this->SDMem->UART->SD_Buffer_Tx[x]=pl_data2[x-16];
          }

          calc_cluster=(12*128)+129+(((calc_cluster-1)-128)*clustersPerMonth);
          pl_data2=(char*)&calc_cluster+3;

          x=21;
          for(;x>19;x--)
          {
            this->SDMem->UART->SD_Buffer_Tx[x]=*pl_data2;
            pl_data2--;
          }

          x=27;
          for(;x>25;x--)
          {
            this->SDMem->UART->SD_Buffer_Tx[x]=*pl_data2;
            pl_data2--;
          }

          for(x=28;x<32;x++)// Reinitialize size of file to zero (empty file)
          {
            this->SDMem->UART->SD_Buffer_Tx[x]=0x00;
          }

          pl_data+=32;

          for(x=0;x<4;x++) // escribe en I2C EEPROM 4 bloques de 128 Bytes competando 512 bytes
          {
            p_loggerMem->loggingData(addrBackupSDMem+(128*x), (char*)&this->SDMem->UART->SD_Buffer_Tx[128*x], 128, p_I2CMem); // Solo para una memoria de 128 KB
          }

          x=12;
          calc_cluster=this->SDMem->UART->SD_Buffer_Tx[x]*0x00000001 | this->SDMem->UART->SD_Buffer_Tx[x+1]*0x00000100 | this->SDMem->UART->SD_Buffer_Tx[x+2]*0x00010000 | this->SDMem->UART->SD_Buffer_Tx[x+3]*0x01000000;

          /*DMA0CTL|=DMAIE;
          DMA1CTL|=DMAIE;
          DMA2CTL|=DMAIE;*/
         // tries=0;

          this->SDMem->startInitializeMem();
          p_I2CMem->enableFlagWritingBckUpSD();
          this->writeFAT((calc_cluster*this->sector_per_cluster)+cluster_begin_lba);
          /*DMA0CTL&=~DMAIE;
          DMA1CTL&=~DMAIE;
          DMA2CTL&=~DMAIE;*/

          /*while(tries<3 && this->SDMem->startInitializeMem())
          {
            p_I2CMem->enableFlagWritingBckUpSD();
            if(this->writeFAT((calc_cluster*this->sector_per_cluster)+cluster_begin_lba)) // Wirtes on cluster for each year folder
            {
              DMA0CTL&=~DMAIE;
              DMA1CTL&=~DMAIE;
              DMA2CTL&=~DMAIE;

              tries=3;
            }
            else
              tries++;
          }*/


        }
      }
      }
    }

    /*}
    else
    {
      // No crear subcarpetas y solo borrar los últimos archivos de todos los meses
    }*/
    //this->writeFAT(this->l_block_sector);
  }// if p_data
  //IE2|=(URXIE1);
}
/****************************************************************************************************/
/* FolderName format: MM/YYYY */
/* Returns cluster pointer to file, returns zero if not was found*/
/* Iteration 0-2 which returns the cluster of the level of the file or folder*/
//unsigned long int CFATDriver::searchFolder(char* FolderName, char iteration, bool fileSizeReq ,CLogger *p_loggerMem, CEEPROM *p_I2CMem)
unsigned long int CFATDriver::searchFolder(char* FolderName, char iteration, char fileDataReq)
{
  int k,x,n_Month;
  int l_pChar;
  char* p_data;
  char* l_p_FolderName=&FolderName[3];

  unsigned long int result=0x00000000;

  for(k=0;k<8;k++)    // 8 represents the qty of blocks (1 cluster) for year folders using 32 bytes per register (128 years)
  {
    l_pChar=0x00;
    p_data=this->SDMem->startReadFromMem(this->cluster_begin_lba+k);

    while(l_pChar<blockSize)
    {
      for(x=0;x<4;x++)
      {
        if(p_data[l_pChar+x]!=l_p_FolderName[x])
        {
          break;
        }
      }
      if(x<4) // Year's name wasn´t found
      {
        l_pChar+=32;
      }
      else
      {// Year's name was found
        if(p_data[l_pChar+11]==0x10) // Attrib ID is a Folder?
        {
          // Gets Folder or File "Next" Cluster Location
          result=p_data[l_pChar+21]*0x01000000 | p_data[l_pChar+20]*0x00010000 | p_data[l_pChar+27]*0x00000100 | p_data[l_pChar+26]*0x00000001;
          k=8;
        }
        break; // breaks from while sentence
      }
    }
  }

  k=0;
  while(iteration)
  {
    iteration--;
    if(result) // Starts to find month folder
    {
        l_pChar=0x00;

        n_Month=(((*(char*)&FolderName[0]) & 0x01) * 10) + ((*(char*)&FolderName[1]) & 0x0F)-1;

        l_p_FolderName=(char*)&table_Months[n_Month][0];

        p_data=this->SDMem->startReadFromMem((result*this->sector_per_cluster)+this->cluster_begin_lba);

        result=0x00000000;
        while(l_pChar<blockSize)
        {
          for(x=0;x<3;x++)
          {
            if(p_data[l_pChar+x]!=l_p_FolderName[x])
            {
              break;
            }
          }
          if(x<3) // Month's name folder wasn´t found
          {
            l_pChar+=32;
          }
          else
          {// Month's name folder was found
            if(k==0)
            {
              x=0x10;
              k++;
            }
            else
            {
              x=0x20;
            }

            if(p_data[l_pChar+11]==x) // Attrib ID is a Folder or File?
            {
              if(fileDataReq)
              {
                  switch(fileDataReq)
                  {
                  case 0x01: // Gets File Size
                            if(x==0x20)
                            {
                              result=(p_data[l_pChar+31]*0x01000000) & 0xFF000000 | (p_data[l_pChar+30]*0x00010000) & 0x00FF0000 | (p_data[l_pChar+29]*0x00000100) & 0x0000FF00 | (p_data[l_pChar+28]*0x00000001) & 0x000000FF;
                            }
                            else
                            {
                              // Gets Folder or File "Next" Cluster Location
                              result=(p_data[l_pChar+21]*0x01000000) & 0xFF000000 | (p_data[l_pChar+20]*0x00010000) & 0x00FF0000 | (p_data[l_pChar+27]*0x00000100) & 0x0000FF00 | (p_data[l_pChar+26]*0x00000001) & 0x000000FF;
                            }
                            break;
                  case 0x02: // Gets the local cluster
                            if(x==0x20)
                            {
                              result=(p_data[l_pChar+15]*0x01000000) & 0xFF000000 | (p_data[l_pChar+14]*0x00010000) & 0x00FF0000 | (p_data[l_pChar+13]*0x00000100) & 0x0000FF00 | (p_data[l_pChar+12]*0x00000001) & 0x000000FF;
                            }
                            else
                            {
                              // Gets Folder or File "Next" Cluster Location
                              result=(p_data[l_pChar+21]*0x01000000) & 0xFF000000 | (p_data[l_pChar+20]*0x00010000) & 0x00FF0000 | (p_data[l_pChar+27]*0x00000100) & 0x0000FF00 | (p_data[l_pChar+26]*0x00000001) & 0x000000FF;
                            }
                            break;
                  }
              }
              else
              {// Gets Folder or File "Next" Cluster Location
                result=(p_data[l_pChar+21]*0x01000000) & 0xFF000000 | (p_data[l_pChar+20]*0x00010000) & 0x00FF0000 | (p_data[l_pChar+27]*0x00000100) & 0x0000FF00 | (p_data[l_pChar+26]*0x00000001) & 0x000000FF;
              }
            }
            break; // breaks from while sentence
          }
        }
    }
  }

  return result;
}

/****************************************************************************************************/
/* FilenameAddress format: MM/YYYY */
/* Returns actual size of file, returns zero if not was found*/
/* Iteration 0-2 which returns the cluster of the level of the file or folder*/
unsigned long int CFATDriver::getSizeOfFile(char* FileNameAddress)
{
  return this->searchFolder(FileNameAddress, 2, 0x01);
}
/****************************************************************************************************/
/* FilenameAddress format: MM/YYYY */
/* Returns cluster pointer to file, returns zero if not was found*/
/* Iteration 0-2 which returns the cluster of the level of the file or folder*/
unsigned long int CFATDriver::searchClusterOfFile(char* FileNameAddress)
{
  return this->searchFolder(FileNameAddress, 2, 0x00);
}
/****************************************************************************************************/
/* FilenameAddress format: MM/YYYY */
/* Returns cluster pointer to file, returns zero if not was found*/
/* Iteration 0-2 which returns the cluster of the level of the file or folder*/
unsigned long int CFATDriver::getLocalClusterOfFile(char* FileNameAddress)
{
  return this->searchFolder(FileNameAddress, 2, 0x02);
}
/****************************************************************************************************/
void CFATDriver::setRegisterOnSDMem(char* FileName, unsigned long int l_NCluster, CMPG2* MPG2, CLogger* loggerSAT, CEEPROM* Mem)
{
  unsigned long int l_fileSize;
  int x,y,j;
  ldiv_t clc_mem;
  char* p_data;
  char* p_CommonReg;
// Gets file size
          //mySystem->m_Fat->SDMem->startInitializeMem();
          l_fileSize=this->getSizeOfFile(FileName);

          clc_mem=ldiv(l_fileSize,blockSize);

           //p_data=mySystem->m_Fat->SDMem->startReadFromMem((l_NCluster*mySystem->m_Fat->sector_per_cluster)+clc_mem.quot+mySystem->m_Fat->cluster_begin_lba);
          p_data=0x0000;
          p_data=this->SDMem->startReadFromMem((l_NCluster*this->sector_per_cluster)+clc_mem.quot+this->cluster_begin_lba);

          if(p_data)
          {
            x=strlen((char*)MPG2->UART->Buffer_Rx);
            // x Contains the number of character to add.
            l_fileSize+=x;
            p_CommonReg=(char*)&l_fileSize;

            if((clc_mem.rem+x)<blockSize)
            {
                for(y=clc_mem.rem;y<=x+clc_mem.rem;y++) // Add new register to backup of memory SD on localized address of the cluster
                {
                  p_data[y]=MPG2->UART->Buffer_Rx[y-clc_mem.rem];
                }
            }
            else
            {  j=0;

                for(y=clc_mem.rem;y<blockSize;y++) // Add new register to backup of memory SD on localized address of the cluster
                {
                  p_data[y]=MPG2->UART->Buffer_Rx[y-clc_mem.rem];
                  j++;
                }

                for(y=0;y<4;y++)
                {
                  //mySystem->loggerSAT->loggingData(addrBackupSDMem+(128*x), (char*)&mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[128*x], 128, mySystem->Mem); // Solo para una memoria de 128 KB
                  loggerSAT->loggingData(addrBackupSDMem+(128*y), (char*)&p_data[128*y], 128, Mem); // Solo para una memoria de 128 KB
                }

                Mem->enableFlagWritingBckUpSD();
                if(this->SDMem->startInitializeMem())
                {
                  this->writeFAT((l_NCluster*this->sector_per_cluster)+clc_mem.quot+this->cluster_begin_lba); // Writes on root folder memory space
                }

                clc_mem.quot++;
                clc_mem.rem=0;

                for(y=clc_mem.rem+j;y<=x+clc_mem.rem;y++) // Add new register to backup of memory SD on localized address of the cluster
                {
                  p_data[y-j]=MPG2->UART->Buffer_Rx[y-clc_mem.rem];
                }
            }

            for(y=0;y<4;y++)
            {
              //mySystem->loggerSAT->loggingData(addrBackupSDMem+(128*x), (char*)&mySystem->m_Fat->SDMem->UART->SD_Buffer_Rx[128*x], 128, mySystem->Mem); // Solo para una memoria de 128 KB
              loggerSAT->loggingData(addrBackupSDMem+(128*y), (char*)&p_data[128*y], 128, Mem); // Solo para una memoria de 128 KB
            }

            Mem->enableFlagWritingBckUpSD();
            if(this->SDMem->startInitializeMem())
            {
              this->writeFAT((l_NCluster*this->sector_per_cluster)+clc_mem.quot+this->cluster_begin_lba); // Writes on root folder memory space
            }

            // Gets the cluster number of local file to increment and stores the new size of file
            this->SDMem->startInitializeMem();
            l_NCluster=this->getLocalClusterOfFile(FileName);
            p_data=this->SDMem->startReadFromMem((l_NCluster*this->sector_per_cluster)+this->cluster_begin_lba);

            for(y=28;y<32;y++)
            {
              p_data[y]=*p_CommonReg++;
            }

            for(y=0;y<4;y++)
            {
              loggerSAT->loggingData(addrBackupSDMem+(128*y), (char*)&p_data[128*y], 128, Mem); // Solo para una memoria de 128 KB
            }

            Mem->enableFlagWritingBckUpSD();
            if(this->SDMem->startInitializeMem())
            {
              this->writeFAT((l_NCluster*this->sector_per_cluster)+this->cluster_begin_lba); // Writes on root folder memory space
            }
          }
}
/****************************************************************************************************/
/* FolderName format: MM/YYYY */
/* Returns "true" if folder was deleted */
bool CFATDriver::searchAndDeleteFolder(char* FolderNameYear, CLogger* loggerSAT, CEEPROM* Mem)
{
  int k,x,y;
  int l_pChar;
  char* p_data;
  char* l_p_FolderName=&FolderNameYear[3];
  bool result = false;

  for(k=0;k<8;k++)    // 8 represents the qty of blocks (1 cluster) for year folders using 32 bytes per register (128 years)
  {
    l_pChar=0x00;
    p_data=this->SDMem->startReadFromMem(this->cluster_begin_lba+k);

    while(l_pChar<blockSize)
    {
      for(x=0;x<4;x++)
      {
        if(p_data[l_pChar+x]!=l_p_FolderName[x])
        {
          break;
        }
      }
      if(x<4) // Year's name wasn´t found
      {
        l_pChar+=32;
      }
      else
      {// Year's name was found
        if(p_data[l_pChar+11]==0x10) // Attrib ID is a Folder?
        {
          // Writes character 0xE5 to flag the root cluster folder as erased
          p_data[l_pChar]=0xE5;
          for(y=0;y<4;y++)
          {
            loggerSAT->loggingData(addrBackupSDMem+(128*y), (char*)&p_data[128*y], 128, Mem); // Solo para una memoria de 128 KB
          }
          Mem->enableFlagWritingBckUpSD();
          if(this->SDMem->startInitializeMem())
          {
            this->writeFAT(this->cluster_begin_lba+k); // Writes on root folder memory space
          }
          result = true;
          k=8;
        }
        break; // breaks from while sentence
      }
    }
  }
  return result;
}
/****************************************************************************************************/
