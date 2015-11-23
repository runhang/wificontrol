#include "lcd1602.h"                                                                          #include "lcd1602.h"   


/***********初始化********************************************************/	
void LCDInit(void)
{
 	LCD_Data = 0;
 	WriteCommandLCD(0x38,0); 	//三次模式设置，不检测忙信号
 	Delay5Ms(); 
 	WriteCommandLCD(0x38,0);
 	Delay5Ms(); 
 	WriteCommandLCD(0x38,0);
 	Delay5Ms(); 

 	WriteCommandLCD(0x38,1); 	//显示模式设置,开始要求每次检测忙信号
 	WriteCommandLCD(0x08,1); 	//关闭显示
 	WriteCommandLCD(0x01,1); 	//显示清屏
 	WriteCommandLCD(0x06,1); 	//显示光标移动设置
 	WriteCommandLCD(0x0C,1); 	//显示开及光标设置
}

/***********按指定位置显示一个字符*******************************************/	
void DisplayOneChar(unsigned char X, unsigned char Y, unsigned char DData)
{
 	Y &= 0x1;
 	X &= 0xF; 				//限制X不能大于15，Y不能大于1
 	if (Y) X |= 0x40; 		//当要显示第二行时地址码+0x40;
 	X |= 0x80; 			//算出指令码
 	WriteCommandLCD(X, 0); //这里不检测忙信号，发送地址码
 	WriteDataLCD(DData);
}

/***********按指定位置显示一串字符*****************************************/	
void DisplayListChar(unsigned char X, unsigned char Y, unsigned char code *DData)
{
 	unsigned char ListLength;

 	ListLength = 0;
 	Y &= 0x1;
 	X &= 0xF; 				//限制X不能大于15，Y不能大于1
 	while (DData[ListLength]>=0x20){ //若到达字串尾则退出
   		if (X <= 0xF){ 		//X坐标应小于0xF
     		DisplayOneChar(X, Y, DData[ListLength]); //显示单个字符
     		ListLength++;
     		X++;
    	}
  	}
}


/***********短延时********************************************************/	
void Delay5Ms(void)
{
 	unsigned int TempCyc = 5552;
 	while(TempCyc--);
}

/***********长延时********************************************************/	
void Delay400Ms(void)
{
 	unsigned char TempCycA = 5;
 	unsigned int TempCycB;
 	while(TempCycA--){
  		TempCycB=7269;
  		while(TempCycB--);
 	}
}

/***********lcd显示函数开始********************************************************/	
void lcd_display(const char* up, const char* down)
{
   
	Delay400Ms(); 	//启动等待，等LCD讲入工作状态
	LCDInit(); 		//初始化
	Delay5Ms(); 	//延时片刻(可不要)                                         +


	DisplayListChar(0, 0, up);
	DisplayListChar(1, 5, down);
	ReadDataLCD();	//测试用句无意义
    Delay400Ms();
    //POWER = 0;
}

/***********写数据********************************************************/	
void WriteDataLCD(unsigned char WDLCD)
{
 	ReadStatusLCD(); //检测忙
 	LCD_Data = WDLCD;
 	LCD_RS = 1;
 	LCD_RW = 0;
 	LCD_E = 0; 		//若晶振速度太高可以在这后加小的延时
 	LCD_E = 0; 		//延时
 	LCD_E = 1;
}

/***********写指令********************************************************/	
void WriteCommandLCD(unsigned char WCLCD,BuysC) //BuysC为0时忽略忙检测
{
 	if (BuysC) ReadStatusLCD(); //根据需要检测忙
 	LCD_Data = WCLCD;
 	LCD_RS = 0;
 	LCD_RW = 0; 
 	LCD_E = 0;
 	LCD_E = 0;
	LCD_E = 1; 
}

/***********读数据********************************************************/	
unsigned char ReadDataLCD(void)
{
 	LCD_RS = 1; 
 	LCD_RW = 1;
 	LCD_E = 0;
 	LCD_E = 0;
 	LCD_E = 1;
 	return(LCD_Data);
}

/***********读状态*******************************************************/	
unsigned char ReadStatusLCD(void)
{
 	LCD_Data = 0xFF; 
 	LCD_RS = 0;
 	LCD_RW = 1;
 	LCD_E = 0;
 	LCD_E = 0;
 	LCD_E = 1;
 	while (LCD_Data & Busy); //检测忙信号
 	return(LCD_Data);
}
