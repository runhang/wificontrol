#ifndef _LCD1602_H
#define _LCD1602_H

#include <reg52.h>

// bit	B_TX1_Busy;	//发送忙标志
/********IO引脚定义***********************************************************/
sbit POWER = P2^4; //写入数据前，高电压，然后低电压
sbit LCD_RS=P2^5;//定义引脚
sbit LCD_RW=P2^6;
sbit LCD_E=P2^7;

/********宏定义***********************************************************/
#define LCD_Data P0
#define Busy    0x80 //用于检测LCD状态字中的Busy标识

/********数据定义*************************************************************/
//unsigned char code uctech[] = {"Happy every day"};

/*************************************************************/
void WriteDataLCD(unsigned char WDLCD);					//写数据
void WriteCommandLCD(unsigned char WCLCD,BuysC);		//写命令
unsigned char ReadDataLCD(void);						//读数据
unsigned char ReadStatusLCD(void);						//读状态
void LCDInit();										//初始化
void DisplayOneChar(unsigned char X, unsigned char Y, unsigned char DData);			//相应坐标显示字节内容
void DisplayListChar(unsigned char X, unsigned char Y, unsigned char code *DData);	//相应坐标开始显示一串内容
void Delay5Ms();									//延时
void Delay400Ms();									//延时
void lcd_display(const char * up, const char * down);  //字符串双行显示


#endif // 