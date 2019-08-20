/********************************************************************
文件名： LM75A.C
功能说明： 本程序使用P89V51RB2单片机，读取LM75A温度值。
使用模拟I2C软件包
更新时间：2010.04.13
********************************************************************/
#include <reg52.h>
#include <intrins.h>
#include "VI2C_C51.h"

#define FOSC    11059200L   //12000000L
#define T10MS   (65536 - FOSC / 12 / 100)   //10ms timer calculation method in 12T mode
#define T2MS    (65536 - FOSC / 12 / 500)
#define BAUD        9600
bit g_flagUartBusy = isNo;
bit g_flagUart = isNo;

#define uchar unsigned char
#define OSCCLK 22118400UL //定义振荡器时钟和处理器时钟频率
#define CPUCLK (OSCCLK / 12)
#define LOW_BYTE(x) (unsigned char)(x)
#define HIGH_BYTE(x) (unsigned char)((unsigned int)(x) >> 8)
uchar LM75 = 0x90; //定义LM75A从机地址
uchar TimerFlag=0;
// uchar code Tos[2] = {0x28,0x00}; //定义LM75A过热关断温度40℃
uchar code Tos[2] = {0x1c,0x80}; //定义LM75A过热关断温度28℃
// uchar code Thyst[2] = {0x23,0x00}; //定义LM75A滞后温度35℃
uchar code Thyst[2] = {0x1c,0x00}; //定义LM75A滞后温度35℃
uchar Temperature[2] = {0};

sbit io_Led = P0 ^ 0;


void delay_ms(uint uiCount)        //@12.000MHz
{
    uchar i, j;
    uint k = 0;

    for (k = 0; k < uiCount; k++)
    {
        i = 2;
        j = 239;
        do
        {
            while (--j);
        } while (--i);
    }
}

// Timer初始化
void timerInit(void) {
    TMOD = 0x01; //设置新模式：16位定时模式
    TL0 = 0xF0;
    TH0 = 0xFF;
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}
// Timer0中断服务函数
void ISR_Timer0(void) interrupt 1
{
    TR0 = 0;
    TL0 = LOW_BYTE(65536UL - CPUCLK / 100); //Timer装载（定时10ms）
    TH0 = HIGH_BYTE(65536UL - CPUCLK / 100);
    TR0 = 1;
    TimerFlag++;
}

// 设定定时器2波特率发生器 自动重装方式
//--------------------------------------------------------------
void uart_Init(void)        //9600bps@11.0592MHz
{
    SCON = 0x50;                    //8位数据,可变波特率
    TL2 = RCAP2L = (65536-(FOSC/32/BAUD)); //Set auto-reload vaule 自动重装值
    TH2 = RCAP2H = (65536-(FOSC/32/BAUD)) >> 8;
    T2CON = 0x34;           //Timer2 start run
    TR2 = 1;        //启动定时器2
    ES = 1;                 //Enable UART interrupt
    //EA = 1;                 //Open master interrupt switch

}

/*----------------------------
UART interrupt service routine
----------------------------*/
void uart_Isr() interrupt 4 using 1     //串口中断程序
{
    if (RI)     //接收到信号：
    {
        RI = 0;             //Clear receive interrupt flag 清除接收标记
    }

    if (TI)     //发送
    {
        TI = 0;             //Clear transmit interrupt flag //清除标志
        g_flagUartBusy = isNo;           //Clear transmit busy flag     //清除忙标志
    }
}


void uart_SendData(uchar ucData)
{
    g_flagUartBusy = isYes;
    SBUF = ucData;
    while(g_flagUartBusy == isYes);
}

void main()
{

    delay_ms(100);
    uart_Init();

    ISendStr(LM75,0x03,Tos,2); //设定LM75A热关断温度值
    ISendStr(LM75,0x02,Thyst,2); //设定LM75A滞后温度值
    timerInit();


    for(;;)
    {
        PCON |= 0x01; //进入空闲模式
        if (TimerFlag == 100) //若10x10=100ms延迟时间到
        {
            TimerFlag = 0; //清除溢出标志
            IRcvStr(LM75,0x00,Temperature,2); //读取LM75A温度
            uart_SendData(Temperature[0]);
            uart_SendData(Temperature[1]);

            io_Led = !io_Led;
        }
    }
}

void testGit(uchar ucData) {
    ucData++;    
}
