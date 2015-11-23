#include "lcd1602.h"  


#define uchar unsigned char
#define uint  unsigned int
#define RX_LENGTH 30
#define BUFFER_LENGTH 30

/*************	本地变量声明	**************/
bit status = 0;//设置一个标志位，串口程序出现指定的字串，并动态过滤到指定的字符后置1
uchar data_string[RX_LENGTH] ; //接收用户纯文本
bit	B_TX1_Busy;	//发送忙标志
uchar ceng = 0;//设置匹配字符串的游标
uchar RSV_MODE = 0;
uchar RSV_PID = 1;   //接收用户发送的信息
uchar RSV_OK  = 2;
uchar RSV_ERROR = 3;
uchar userId = 0; //接到模块的用户id
uchar text_length = 0;//用户发过来的纯文本的长度
uchar text_num; //记录用户文本的字符个数；
uchar flags = 0; //字符标志位
uchar vvalue = 0;

//Led数码管数
sbit DIO = P2^0;				//串行数据输入
sbit LRCLK = P2^1;				//时钟脉冲信号——上升沿有效
sbit LSCLK = P2^2;				//打入信号————上升沿有效


unsigned char code LED_0F[] = 
{// 0	 1	  2	   3	4	 5	  6	   7	8	 9	  A	   b	C    d	  E    F    -
	0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0x8C,0xBF,0xC6,0xA1,0x86,0xFF,0xbf
};


const uchar * MODE_STRING = "AT+MODE=2\r\n";
const uchar * MUX_STRING = "AT+CIPMUX=1\r\n";
uchar const * SERVER_STRING = "AT+CIPSERVER=1,5000\r\n";
uchar string_length(char* sub_string);  //返回字符串长度
const uchar *FB_PID = "+IPD,";
const uchar *FB_OK = "OK";
const uchar *FB_ERROR = "ERROR";


uchar scan_keyboard(void);   // 扫描键盘
void delay_ms(int n);       //毫秒级延时函数
void get_LED(uchar key);    //获得LED对应的数组值
void LED_ON(uchar x);      //点亮led数码管
void uart_init();
void send_string(const char* c_string);     //发送字符串
void receive_string();               //接收字符串
void clear_buffer(uchar *s);      //清除缓存数组
uchar match_string(const char * f_string, const char * s_string);      //匹配收到的字符串
void clone_string(char* s, char* sc, uchar len); //赋值有效字符串
/********IO引脚定义***********************************************************/
//sbit POWER = P2^4; //写入数据前，高电压，然后低电压
//sbit LCD_RS=P2^5;//定义引脚
//sbit LCD_RW=P2^6;
//sbit LCD_E=P2^7;
//电机引脚定义
sbit motor1 = P2^3;
sbit motor2 = P2^4;

/********宏定义***********************************************************/
//#define LCD_Data P0
//#define Busy    0x80 //用于检测LCD状态字中的Busy标识

/********数据定义*************************************************************/
unsigned char code uctech[] = {"Happy every day"};
/*************************************************************/
void motor_forward();//电机正转
void motor_reverse();//电机逆转
void motor_stop();//电机停止
//电机正转
void motor_forward()
{
    motor2 = 0;
    motor1 = 1;
}
//电机逆转
void motor_reverse()
{
    motor1 = 0;
    motor2 = 1;
}
//电机停止
void motor_stop()
{
    motor1 = 0;
    motor2 = 0;
}

/*主函数*/
void main()
{   
    
    uchar key = 0X00;  //此处要给初值
    uchar x = 0x01;
    uchar y = 0x05;
    uchar k = 0;
    //P1 = 0xf0;              //行线输出全为0,列线输出全为1 
    uart_init();
    //clear_buffer(data_string);
    while(1){

       
       if(flags == 1)
       {
        //串口数据LCD显示
          x = 0x01;
          Delay400Ms(); 	//启动等待，等LCD讲入工作状态
	      LCDInit(); 		//初始化
	      Delay5Ms(); 	//延时片刻(可不要) 
          DisplayListChar(0, 0, uctech);
          y &= 0x01;
 	      x &= 0xF;
          send_string(&vvalue);
          while(k < vvalue) {
   		     if (x <= 0xF){ 		//X坐标应小于0xF
     	        DisplayOneChar(x, y, data_string[k]); //显示单个字符
     	        k++;
                x++;
               }            
             } 
          vvalue = 0;   
          flags = 0;
          k = 0;
       }
       key = scan_keyboard();   
       get_LED(key);
      
    }
}


/**
* 串口初始化
*/
void uart_init()
{
    SCON  = 0x50;		        /* SCON: 模式 1, 8-bit UART, 使能接收         */
    TMOD |= 0x20;               /* TMOD: timer 1, mode 2, 8-bit reload        */
    TH1   = 0xFD;               /* TH1:  reload value for 9600 baud @ 11.0592MHz   */
    TL1 = 0XFD;
    EA    = 1;                  /*打开总中断*/
    ES    = 1;                  /*打开串口中断*/
    TR1   = 1;                  /* TR1:  timer 1 run   启动波特率发生     */
}

/**
*   延时函数
*/
void delay_ms(int n)
{
    int i = 0, j = 0;
    for(i = 0; i < n; i++)
        for(j = 0; j < 100; j++);
}

//开灯
void LED_ON(uchar lcode)
{
    uint i;
	for(i = 8;i >= 1; i--)
	{
        if (lcode & 0x80) 
            DIO = 1; 
        else 
            DIO = 0;
        lcode <<= 1;
	    LSCLK = 0;
	    LSCLK = 1;
	}
}
                 
void LED_code(uchar value)
{
    uchar code *led_table;          // 查表指针
	uchar lcode;               //要得到的显示码
	//显示第1位
/*	led_table = LED_0F + value;
	lcode = *led_table;

	LED_ON(lcode);			
	LED_ON(0x0f);
    
    LRCLK = 0;
	LRCLK = 1;
    //显示第2位
	led_table = LED_0F + value;
	lcode = *led_table;

	LED_ON(lcode);			
	LED_ON(0x0f);
    
    LRCLK = 0;
	LRCLK = 1;
    //显示第3位
	led_table = LED_0F + value;
	lcode = *led_table;

	LED_ON(lcode);			
	LED_ON(0x0f);
    
    LRCLK = 0;
	LRCLK = 1; */
    //显示第4位
	led_table = LED_0F + value;
	lcode = *led_table;

	LED_ON(lcode);			
	LED_ON(0x0f);
    
    LRCLK = 0;
	LRCLK = 1;     
}

void get_LED(uchar key)
{
    if(key == 0x00)
        return;   //如果为0x00，说明没有扫描到值，返回
    switch(key)
    {
        case 0xee: 
            LED_code(0); 
            send_string(MODE_STRING);
            lcd_display(uctech, "MODE ok");
            break;//0按下相应的键显示相对应的码值
		case 0xde: 
            LED_code(1);   
            send_string(MUX_STRING); 
            lcd_display(uctech, "CIP ok");
            //send_string(MUX_STRING);
            break;//1 按下相应的键显示相对应的码值 
		case 0xbe: 
            LED_code(2);
            send_string(SERVER_STRING);
            lcd_display(uctech, "Server ok");
            //send_string(SERVER_STRING); 
            break;//2
		case 0x7e: 
            LED_code(3);
            break;//3
		case 0xed: 
            LED_code(4); 
            break;//4
		case 0xdd: 
            LED_code(5); 
            break;//5
		case 0xbd: 
            LED_code(6);
            lcd_display(uctech, "runhang");
            break;//6
		case 0x7d: 
            LED_code(7); 
            break;//7
		case 0xeb: 
            LED_code(8); 
            motor_forward(); 
            break;//8
		case 0xdb: 
            LED_code(9); 
            motor_reverse();
            break;//9
		case 0xbb: 
            LED_code(10); 
            motor_stop();
            break;//a
		case 0x7b: 
            LED_code(11); 
            break;//b
		case 0xe7: 
            LED_code(12); 
            break;//c
		case 0xd7: 
            LED_code(13);  
            break;//d
		case 0xb7: 
            LED_code(14); 
            break;//e
		case 0x77: 
            LED_code(15); 
            break;//f
    }
}

//  扫描键盘
uchar scan_keyboard()
{
    uchar row_wire, col_wire;    //定义行线和列线，行线0-3，列线4-7
    //uchar flags;
    P1 = 0xf0;              //行线输出全为0,列线输出全为1 
    col_wire = P1 & 0xf0;   //读入列线值
    //if(flag != '1')
      //  return 0x00;
    if(col_wire != 0xf0)
    {
        delay_ms(1);  //去抖延时
        if(col_wire != 0xf0)  
        {
            col_wire = P1 & 0xf0;  //读入列线值
            P1 = col_wire | 0x0f;  //输出当前列线值
            row_wire = P1 & 0x0f;  //读入行线值
            P1 = 0xf0;
            while((P1&0xf0) != 0xf0);
            return(col_wire + row_wire);//键盘最后组合码值
        }
    }
    return 0x00;
}

/*
// 发送字符串
void send_string(char  * c_string)
{
    unsigned char i = 0;
    ES = 0;
    while(c_string[i] != '\0')
    {
        SBUF = c_string[i];
        while(!TI);		  //等待发送数据完成
        i++;
		TI=0;  
    }
    delay_ms(50);		  //延时一下再发
    ES = 1;

} */


// 发送字符串
void send_string(const uchar * c_string)
{
    ES = 0;
    while(*c_string != '\0')
    {
        SBUF = *c_string;
        while(!TI);        //等待发送数据完成
        TI = 0;
        c_string++;
    }
   ES = 1;
}



//比较用户发送过来的字符，判断哪儿种模式
void fb_pid(uchar stmp)
{   
    if(ceng <= 4)
    {
        if(stmp == FB_PID[ceng])
        {
            ceng++;    
        }else
        {
            ceng = 0;
            RSV_MODE = 0;
        }
    }else if(ceng == 5)  
    {
        userId = stmp;
        ceng++;
    }/*else if(stmp == ',' && ceng == 6)
    {
        ceng++;    
    }*/else if(ceng == 7)
    {   
        //text_length = stmp - 1;//#/r/n结尾的形式
        text_length = stmp - 0x30;//减去0->对应ascii码为0x30
        ceng++;
    }else if(ceng == 6)
    {
        ceng++;
    }else if(ceng == 8)//(stmp == ':' && ceng == 8)
    {  // send_string(&stmp);
        if(stmp == ':')  //说明字符小于9
        {
           //send_string(&stmp);
           ceng = 0;
           status = 1; 
        }else
        {
            text_length = stmp - 0x26;// 减20；
            ceng++;
        }
        //ceng = 0;
        //status = 1;
        //RSV_MODE = 0;       
    }else if(ceng == 9)
    {
        if(stmp == ':')
        {
            ceng = 0;
            status = 1;
        }else{       //可以作为扩展
            ceng = 0;
            RSV_MODE = 0;
        }
    }else{
        send_string("errorpid");
            ES = 0;
            RI = 0;
            SBUF = ceng;
            while(!TI);
            TI = 0;
            ES = 1;
        ceng = 0;
        RSV_MODE = 0;
    }
    
/*
    if(stmp == '+' && ceng == 0)
    {
        buffer_string[ceng] = stmp;
        ceng++;   
    }else if(stmp == 'I' && ceng == 1)
    {
        buffer_string[ceng] = stmp;
        ceng++;
    }else if(stmp == 'P' && ceng == 2)
    {
        buffer_string[ceng] = stmp;
        ceng++;
    }else if(stmp == 'D' && ceng == 3)
    {
        buffer_string[ceng] = stmp;
        ceng++;
    }else if(stmp == ',' && (ceng == 4 ||ceng == 6))
    {
        buffer_string[ceng] = stmp;
        ceng++;
    }else if(stmp <= 0x39 && stmp >= 0x30 && (ceng == 5 ||ceng == 7) ) */ /*ascii码*/
     /*{
        buffer_string[ceng] = stmp;
        ceng++;
    }else if(stmp == ':' && ceng == 8)
    {
        buffer_string[ceng] = stmp;
        ceng++;
        status = 1;        
    } else{
        ceng = 0;
    }*/
   }

//匹配反馈的ERROR字符
void fb_error(uchar stmp)
{
    if(ceng <= 4)
    {
        if(stmp == FB_ERROR[ceng])
        {
            //buffer_string[ceng] = stmp;
            ceng++;    
        }else{
            ceng = 0;
            RSV_MODE = 0;    
        }
    }
    if(ceng > 4)
    {
        send_string(FB_ERROR);// 发送buffer_string 测试使用
        ceng = 0;
        RSV_MODE = 0;
    }
    /*
     if(stmp == 'E' && ceng == 0)
    {
        buffer_string[ceng] = stmp;
        ceng++;        
    }else if(stmp == 'R' && (ceng == 1 || ceng == 2 ||ceng == 4))
    {
        buffer_string[ceng] = stmp;
        if(ceng == 4)
            status = 1;
        else 
            ceng++;
    }else if(stmp == 'O' && ceng == 3)
    {
        ceng++;      
    }else{
        ceng = 0;
    } */        
}


//比较模块OK的字符
void fb_oK(uchar stmp)
{
    if(ceng <= 1)
    {
        if(stmp == FB_OK[ceng])
        {
           // buffer_string[ceng] = stmp;
            ceng++;     
        }else{
            ceng = 0;
            RSV_MODE = 0;
        }
    }
    if(ceng > 1){//数据接收完成
        send_string(FB_OK);// 发送buffer_string 测试使用
        ceng = 0;
        RSV_MODE = 0;
    }

  /*  if(stmp == 'O' && ceng == 0)
    {
        buffer_string[ceng] = stmp;
        ceng++;        
    }else if(stmp == 'K' && ceng == 1)
    {
        buffer_string[ceng] = stmp;
        status = 1;
    }else{
        ceng = 0;
    }   */    
}



 // 接收字符
void receive_char() interrupt 4
{  /*
    uchar stmp = 0;
    stmp = SBUF;
    RI = 0;
    //BUFFER_STRING[i] = stmp;
    SBUF = stmp;
    while(!TI);		  //等待发送数据完成
	    TI=0;			  //清除发送完成标志位
   */ 
   if(status == 1)
    {   
        RI = 0;
       /* if(SBUF != '#')
        {   
            data_string[vvalue] = SBUF;
            vvalue++;
            RI = 0;
        }
        if(SBUF == '#')
        {   
            uchar i;
            flags = 1; /*
            for(i = 0; i <= vvalue; i++)
            {
                ES = 0;
                SBUF = data_string[i];
                while(!TI);
                    TI = 0;
                ES = 1;
            }  */
            //send_string(&vvalue);   
            //data_string[ceng] ='\0';
            //vvalue = ceng;
            //ceng = 0;
         /*   status = 0;
            RSV_MODE = 0;     
            //电机控制
            if(data_string[0] == 'M' && vvalue > 1)
            {
             if(data_string[1] == '1')  //电机正转
                {      
                    LED_code(8);
                    motor_forward();//
                }else if(data_string[1] == '2'){ //电机反转
                    motor_reverse();
                    LED_code(9);
                }else{ //建议输入3  电机停止
                    motor_stop();
                    LED_code(10);
                }
                
            }else{
                LED_code(11);
            }                     
        } 
          */

        
        
        
        //uchar i =0;
        if(ceng < text_length)
        {   
            data_string[ceng] = SBUF;
            //ES = 0;
            RI = 0;
            //SBUF = data_string[ceng]; 
           // while(!TI);
            ceng++;
            //TI = 0;
           // ES = 1;
        }
        if(ceng >= text_length)
        {   
            uchar i = 0;
            flags = 1;/*
            for(i = 0; i < text_length; i++)
            {
                ES = 0;
                RI = 0;
                SBUF = data_string[i]; 
                while(!TI);
                TI = 0;
                ES = 1;
            }*/
            vvalue = text_length;
            if(data_string[0] == 'M' && text_length > 1)
            {
             if(data_string[1] == '1')  //电机正转
                {      
                    LED_code(8);
                    motor_forward();//
                }else if(data_string[1] == '2'){ //电机反转
                    motor_reverse();
                    LED_code(9);
                }else{ //建议输入3  电机停止
                    motor_stop();
                    LED_code(10);
                }
                
            }else{
                LED_code(11);
            }
           // clear_buffer(data_string);
            status = 0;
            RSV_MODE = 0;
            ceng = 0;
       }  
    }else{

        if(RSV_MODE == 0) //对第一个字符串设置模式
        {
            if(SBUF == 'O')
            {
                RSV_MODE = RSV_OK;
                fb_oK(SBUF);

            }else if(SBUF == '+')
            {
                RSV_MODE = RSV_PID;
                fb_pid(SBUF);
            }else if(SBUF == 'E')
            {
                RSV_MODE = RSV_ERROR;
                fb_error(SBUF);
            }
        }else if(RSV_MODE == RSV_OK)
        {
            fb_oK(SBUF);       
        }else if(RSV_MODE == RSV_ERROR)
        {
            fb_error(SBUF);
        }else if(RSV_MODE == RSV_PID)
        {
            fb_pid(SBUF);    
        }

    }
    RI = 0;  
   
}
  
  
  
 //uchar buf;  
    /*if(status)
    {
        if(ceng < buffer_string[7])
            data_string[7] = SBUF;
        else
            send_string(data_string);
        
       
    }else{

        if(RSV_MODE == 0) //对第一个字符串设置模式
        {
            if(SBUF == 'O')
            {
                RSV_MODE = RSV_OK;
                fb_oK(SBUF);

            }else if(SBUF == '+')
            {
                RSV_MODE = RSV_PID;
                fb_pid(SBUF);
            }else if(SBUF == 'E')
            {
                RSV_MODE = RSV_ERROR;
                fb_error(SBUF);
            }
        }else if(RSV_MODE == RSV_OK)
        {
            fb_oK(SBUF);       
        }else if(RSV_MODE == RSV_ERROR)
        {
            fb_error(SBUF);
        }else if(RSV_MODE == RSV_PID)
        {
            fb_pid(SBUF);    
        }

    }
    RI = 0;  
    */

