#include <lcddriver.h>
#include <FreeRTOS.h>
#include <task.h>

/// low level routine
void hardware_init()
{
    ODCCbits.ODCC4 = 1;
    ODCCbits.ODCC5 = 1;
    ODCCbits.ODCC6 = 1;
    ODCCbits.ODCC7 = 1;
    ODCCbits.ODCC8 = 1;
    ODCCbits.ODCC9 = 1;
    LCD_RS_TRIS = 0;
    LCD_E_TRIS = 0;
    LCD_D4_TRIS = 0;
    LCD_D5_TRIS = 0;
    LCD_D6_TRIS = 0;
    LCD_D7_TRIS = 0;
    LCD_E = 0;
    LCD_RS = 0;
}

void write_nibble(char nibble)
{
    LCD_D4 = nibble & 0x01;
    LCD_D5 = (nibble>>=1) & 0x01;
    LCD_D6 = (nibble>>=1) & 0x01;
    LCD_D7 = (nibble>>=1) & 0x01;
}

void lcd_set_e(){LCD_E = 1;}
void lcd_clr_e(){LCD_E = 0;}
void lcd_set_rs(){LCD_RS = 1;}
void lcd_clr_rs(){LCD_RS = 0;}

enum{CMD = 0, DATA = 1};

void LCD_Send(char command, int cw)
{
    lcd_clr_e();
    if(cw) lcd_set_rs();
    else    lcd_clr_rs();
    write_nibble(command >> 4);
    lcd_set_e();
    vTaskDelay(5);
    lcd_clr_e();
    vTaskDelay(5);
    write_nibble(command);
    vTaskDelay(1);
    lcd_set_e();
    vTaskDelay(5);
    lcd_clr_e();
}

void LCD_Init()
{
    hardware_init();
    vTaskDelay(50); /// wait 20ms for startup
    lcd_clr_e();
    write_nibble(0x03);
    lcd_set_e();
    vTaskDelay(5);
    lcd_clr_e();
    write_nibble(0x03);
    lcd_set_e();
    vTaskDelay(1);
    lcd_clr_e();
    write_nibble(0x03);
    lcd_set_e();
    vTaskDelay(1);
    lcd_clr_e();
    write_nibble(0x02);
    lcd_set_e();
    vTaskDelay(5);
    LCD_Send(LCD_SET_INTERFACE_4BIT|LCD_SET_INTERFACE_2LINES|LCD_SET_INTERFACE_5x10DOT,CMD);
    LCD_Send(LCD_DISPLAY_ON,CMD);
    LCD_Send(LCD_CLEAR,CMD);
}

LCD_GotoXY(int row, int col)
{
    if(row==1) col = col - 1;
    if(row==2) col+=0x39;
    /* Add these if you are using LCD module with 4 columns
    if(row==2) column+=0x14;
    if(row==3) column+=0x54;
    */
    LCD_Send(0x80|col,CMD);
}

LCD_PutChar(char c)
{
    LCD_Send(c,DATA);
}

LCD_PutStr(char* str)
{
    while(*str!='\0')
    {
        LCD_PutChar(*str);
        str++;
    }
}