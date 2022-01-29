#include <lpc214x.h>
#include "LPC2148_delay.h"

#define	RDA	0x04

char RX_CHECK,GPS_DATA[350],GPS_DATA_2[350],GPS_DATA_3[50],device_id[16]="IZ09";
int i,j,k,h;

void UART1_init(void)
{
	PINSEL0 = PINSEL0 | 0X00050000;
	U1LCR = 0X83;
	U1DLL = 97;			//0x61
	U1LCR = 0X03;
	U1FCR = 0x01;	
	U1IER = 0x01; // enable RDA interrupt
}
void UART1_Tx(char x)
{
	while(!(U1LSR & 0X20));
	U1THR = x;
}
void UART1_TX_string(char *Str)
{
	while(*Str)
	{
		UART1_Tx(*Str++);
	}
}
void ENTER_1()
{
   UART1_Tx(0x0D);
   UART1_Tx(0x0A);
}
void UART1_Interrupt(void)__irq 
{
  if(U1IIR & RDA);
	h = U1RBR;
	GPS_DATA[i] = h;
	i++;
    VICVectAddr = 0x00;
}
void initClocks(void)
{
        PLL0CON = 0x01;   //Enable PLL
        PLL0CFG = 0x24;   //Multiplier and divider setup
        PLL0FEED = 0xAA;  //Feed sequence
        PLL0FEED = 0x55;

        while(!(PLL0STAT & 0x00000400)); //is locked?

        PLL0CON  = 0x03;   //Connect PLL after PLL is locked
        PLL0FEED = 0xAA;  //Feed sequence
        PLL0FEED = 0x55;
        VPBDIV   = 0x00;    // PCLK is 1/4 as CCLK i.e.15 MHz
}
void GPS_Initialization()
{
	UART1_TX_string("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
	delay(10);ENTER_1();delay(100);

	UART1_TX_string("AT+SAPBR=3,1,\"APN\",\"RCMNET\"");
	delay(10);ENTER_1();delay(100);

	UART1_TX_string("AT+SAPBR=1,1");
	delay(10);ENTER_1();delay(1000);

	UART1_TX_string("AT+SAPBR=2,1");
	delay(10);ENTER_1();delay(1000);
	
	UART1_TX_string("AT+CIPGSMLOC=1,1");
	delay(10);ENTER_1();delay(2000);
}
void Send_To_The_Server()
{
	UART1_TX_string("AT+HTTPINIT");
	delay(10);ENTER_1();delay(100);

	UART1_TX_string("AT+HTTPPARA=\"CID\",1");
	delay(10);ENTER_1();delay(100);

	for(i=0;i<350;i++)
	{
		if(GPS_DATA[i] == '\"')
		{
			continue;
		}
		GPS_DATA_2[j] = GPS_DATA[i];
		j++;
	}
	j=0;
	for(j=0;j<350;j++)
	{
		if((j>150) && (j<190))
		{
			GPS_DATA_3[k] = GPS_DATA_2[j];
			k++;
		}
	}
	UART1_TX_string("AT+HTTPPARA=\"URL\",\"http://iotroztech.net/final_bus/vc.php?device_id=");
	UART1_TX_string(GPS_DATA_3);
//	UART1_TX_string("&&device_id=");
//	UART1_TX_string(device_id);
	UART1_TX_string("\"");
	delay(10);ENTER_1();delay(2000);

	UART1_TX_string("AT+HTTPACTION=0");
	delay(10);ENTER_1();delay(2000);

	UART1_TX_string("AT+SAPBR=0,1");
	delay(10);ENTER_1();delay(500);

	UART1_TX_string("AT+HTTPTERM=0");
	delay(10);ENTER_1();delay(500);
}

int main()
{	
	initClocks();
	UART1_init();

  VICVectAddr0 = (unsigned long int) UART1_Interrupt;
  VICVectCntl0 = 0x27;       
  VICIntEnable |= 0x00000080;
	VICIntSelect = 0x00000000;
	
//	UART1_TX_string("ATE1");
//	delay(1000);ENTER_1();delay(500);

	while(1)
	{
		i=0,j=0,k=0;
		
		GPS_Initialization();
		Send_To_The_Server();
	}
}
