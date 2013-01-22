/* 
 * File:   lcddriver.h
 * Author: diego
 *
 * Created on 13. Oktober 2012, 00:08
 */

#ifndef LCDDRIVER_H
#define	LCDDRIVER_H
#include <p32xxxx.h>
#ifdef	__cplusplus
extern "C" {
#endif
/// PIN SELECTIONS
#define LCD_RS      LATCbits.LATC8
#define LCD_RS_TRIS TRISCbits.TRISC8

#define LCD_E       LATCbits.LATC9
#define LCD_E_TRIS  TRISCbits.TRISC9

#define LCD_D4      LATCbits.LATC4
#define LCD_D4_TRIS TRISCbits.TRISC4

#define LCD_D5      LATCbits.LATC5
#define LCD_D5_TRIS TRISCbits.TRISC5

#define LCD_D6      LATCbits.LATC6
#define LCD_D6_TRIS TRISCbits.TRISC6
    
#define LCD_D7      LATCbits.LATC7
#define LCD_D7_TRIS TRISCbits.TRISC7

#define LCD_CLEAR   0x01
#define LCD_HOME    0x02
#define LCD_CURSOR_MODE_LEFT    0x04
#define LCD_CURSOR_MODE_RIGHT   0x06
#define LCD_DISPLAY_SHIFT       0x05
#define LCD_DISPLAY_NO_SHIFT    0x04
#define LCD_DISPLAY_ON          0x0C
#define LCD_DISPLAY_OFF         0x08
#define LCD_DISPLAY_SHOW_CURSOR       0x0A
#define LCD_DISPLAY_BLINK_CURSOR    0x09

#define LCD_SET_INTERFACE_4BIT   0x1C
#define LCD_SET_INTERFACE_2LINES 0x18
#define LCD_SET_INTERFACE_5x10DOT   0x14

void LCD_Init();
void LCD_PutChar(char c);
void LCD_Clear();
void LCD_GotoXY(int row, int col);
void LCD_PutStr(char* str);

#ifdef	__cplusplus
}
#endif

#endif	/* LCDDRIVER_H */

