
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include <c8051f320.h>                 // SFR declarations
#include <string.h>
//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------

#define BAUDRATE        9600           // Baud rate of UART in bps
#define SYSTEMCLOCK 12000000           // SYSCLK frequency in Hz

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
//unsigned char ch;
bit recdat;
sbit P13 = P1^3; 
sbit P14 = P1^4; 
sbit P15 = P1^5; 
unsigned int flag;
unsigned int icount;

unsigned char inbuf[3] = {0};
unsigned char uart_fin = 0;
unsigned char panduan;
unsigned char posit; //将来要去的位置
unsigned char preposit;//我现在的位置
unsigned char cw;  //控制正转反转


//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------

void SYSCLK_Init (void);
void UART0_Init (void);
void PORT_Init (void);
void Timer0_Init(void);
void Ext_Interrupt_Init(void);

///-----------------------------------------------------------------------------
// MAIN Routine
//-----------------------------------------------------------------------------
void delay(unsigned int N)
{
	unsigned int i;
	unsigned int j;	
	for(i=0;i<N;i++)
	{
		for(j=0;j<125;j++);
	}
}

void serial()interrupt 4   //接收为多个字符
{	
	static unsigned int count = 0; 
	if(RI0==1)
	{			
		RI0 = 0;
		inbuf[count]=SBUF0;
		count++;
		if(count >= 3)
		{
			panduan = inbuf[0];
			posit = inbuf[1];
			cw = inbuf[2];
			count = 0;
			uart_fin = 1;
		}
	}
}

void transmitchar(unsigned char dat)	  //单个字符发送
{
	SBUF0 = dat;
	while(!TI0);
	TI0 = 0;
}

					 
void main (void)
{
	unsigned char preposit ='1';
	PCA0MD &= ~0x40;                    // WDTE = 0 (clear watchdog timer

	Ext_Interrupt_Init ();			    //先后顺序很重要										// enable)
	Timer0_Init ();    
	PORT_Init();                        // Initialize Port I/O
	SYSCLK_Init ();                     // Initialize Oscillator
	UART0_Init();
	
	EA = 1;

	
	ES0 = 1;

	while(1)
	{
			unsigned int jishu=0;	
			unsigned int i=0;
			if(uart_fin == 1)
			{	
				if(panduan=='a')//自动复位	功能1
				{	
					flag = 0;
					P14	= 0;
					P14 = ~P14;	
					while(flag!=1)
					{								
						icount = 170;	
						ET0 = 1;   
						while(icount);
						P13 = ~P13;						
					}
					preposit ='1';	//每回复位之后都要重定义一下当前位置为1						
				}

				if(panduan=='c')//自动旋转照相	 功能2
				{
					flag = 0;
					jishu = 0;
					P14	= 0;
					P14 = ~P14;	
					while(jishu!=25500)
					{								
						icount = 150;	
						ET0 = 1;   
						while(icount);
						P13 = ~P13;
						jishu = jishu+1;
						if(jishu%6375==0)
						{
							P13 = 0;
							delay(10000);
							delay(5000);
						}
					
					}
					///////////
					flag = 0;
					while(flag!=1)
					{								
						icount = 170;	
						ET0 = 1;   
						while(icount);
						P13 = ~P13;						
					}
					/////////////

					RSTSRC &=~0xEF;
					preposit ='1';	//每回复位之后都要重定义一下当前位置为1	
				}

				if(panduan=='f') //获取4个位置	  功能3
				{				
					if(cw=='1')		//cw=1为正转  逆时针
					{
						P14	= 0;
						P14 = ~P14;				 
					    //这里算有关6375的倍数问题，这样就能控制它走的距离
						//posit为想去的位置，preposit为当前位置
						if(posit>preposit)
						{
							jishu = (posit-preposit)*6375;
						}					   
						else
						{
							jishu = (posit+4-preposit)*6375;
						}
					}						
					else		  //cw=0为反转 顺时针
					{					
						P14 = 0;	
						if(preposit>posit)
						{
							jishu = (preposit-posit)*6375;
						}					   
						else
						{
							jishu = (preposit+4-posit)*6375;
						}
					}
				
					while(i<=jishu)	  //6375的倍数
					{
						icount = 150;	 //脉冲
						ET0 = 1;   
						while(icount);
						P13 = ~P13;	
						i=i+1; 
					}
				
					preposit = posit;
				}	
				delay(10000);
				delay(5000);	      	
				transmitchar('o');
				transmitchar('k');
				uart_fin = 0;
			}					
//		}
	}	
}
//-----------------------------------------------------------------------------
// Initialization Subroutines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PORT_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Configure the Crossbar and GPIO ports.
//
// P0.4   digital   push-pull    UART TX
// P0.5   digital   open-drain   UART RX
//
//-----------------------------------------------------------------------------

void PORT_Init (void)
{
   P0MDOUT |= 0x10;                    // Enable UTX as push-pull output
   XBR0    = 0x01;                     // Enable UART on P0.4(TX) and P0.5(RX)
   XBR1    = 0x40;                     // Enable crossbar and weak pull-ups
}

//-----------------------------------------------------------------------------
// SYSCLK_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// This routine initializes the system clock to use the internal oscillator
// at its maximum frequency.
// Also enables the Missing Clock Detector.
//-----------------------------------------------------------------------------

void SYSCLK_Init (void)
{
   OSCICN |= 0x03;                     // Configure internal oscillator for                                       // its maximum frequency
   CLKSEL = 0x20;
   RSTSRC  = 0x04;                     // Enable missing clock detector

}

//-----------------------------------------------------------------------------
// UART0_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Configure the UART0 using Timer1, for <BAUDRATE> and 8-N-1.
//-----------------------------------------------------------------------------


void UART0_Init (void)
{
   SCON0 = 0x10;                       // SCON0: 8-bit variable bit rate
                                       //        level of STOP bit is ignored
                                       //        RX enabled
                                       //        ninth bits are zeros
                                       //        clear RI0 and TI0 bits
   if (SYSTEMCLOCK/BAUDRATE/2/256 < 1) //定时器1时钟源不分频时的最小溢出频率的1/2小于波特率
	{
		TH1 = -(SYSTEMCLOCK/BAUDRATE/2);
		CKCON &= ~0x0B;                  // T1M = 1; SCA1:0 = xx
		CKCON |=  0x08;
	}
	else if (SYSTEMCLOCK/BAUDRATE/2/256 < 4) //时钟源4分频时的最小溢出频率的1/2小于波特率
	{
		TH1 = -(SYSTEMCLOCK/BAUDRATE/2/4);
		CKCON &= ~0x0B;                  // T1M = 0; SCA1:0 = 01		
		CKCON |=  0x05;

	} 
	else if (SYSTEMCLOCK/BAUDRATE/2/256 < 12)
	{
		TH1 = -(SYSTEMCLOCK/BAUDRATE/2/12);
		CKCON &= ~0x0B;                  // T1M = 0; SCA1:0 = 00
	} 
	else 
	{
		TH1 = -(SYSTEMCLOCK/BAUDRATE/2/48);
		CKCON &= ~0x0B;                  // T1M = 0; SCA1:0 = 10
		CKCON |=  0x02;
	}
   TL1 = TH1;                          // init Timer1
   TMOD = 0xa1;						   // TMOD: timer 1 in 8-bit autoreload
   TR1 = 1;                            // START Timer1
   IP |= 0x10;                         // Make UART high priority
   ES0 = 1;                            // Enable UART0 interrupts
}
void Timer0_Init(void)
{
   ET0 = 1;                            // Timer0 interrupt enabled
   TCON |= 0x10;                        // Timer0 ON
}
void Timer0_ISR (void) interrupt 1
{

	TH0 = (65536-10)/256; 			//定时1ms中断
	TL0 = (65536-10)%256;
	if(icount)
	{	
 	  	icount--;
	}
	else
	{
		ET0 = 0;
	}
}
void Ext_Interrupt_Init (void)
{
	TCON |= 0x05;                        // /INT 0 and /INT 1 are edge triggered
	TCON &= ~0xFD;						//101 0x05
	IT01CF = 0x10;                      // /INT0 active low; /INT0 on P0.0;
                                       // /INT1 active low; /INT1 on P0.1
	IT0 = 1; 
	EX0 = 1;                            // Enable /INT0 interrupts
}
void INT0_ISR (void) interrupt 0	   //外部中断
{
	flag = 1;
}	
