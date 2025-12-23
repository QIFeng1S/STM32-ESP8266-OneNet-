#ifndef __OLED_H__
#define __OLED_H__

#include "OLED_data.h"

#define OLED_I2C_4      //使用4线I2C接口
//#define OLED_SPI_7    //使用7线SPI接口

/*参数宏定义*********************/

/*FontSize参数取值*/
/*此参数值不仅用于判断，而且用于计算横向字符偏移，默认值为字体像素宽度*/
#define OLED_8X16				8
#define OLED_6X8				6

/*IsFilled参数数值*/
#define OLED_UNFILLED			0
#define OLED_FILLED				1

/*********************参数宏定义*/

void OLED_Init(void);
void OLED_SetCursor(uint8_t x, uint8_t page);

void OLED_Clear(void);
void OLED_ClearArea(uint8_t x, uint8_t y, uint8_t Width, uint8_t Height);
void OLED_Update(void);
void OLED_UpdateArea(int16_t x, int16_t y, uint8_t Width, uint8_t Height);
void OLED_Reverse(void);
void OLED_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

void OLED_ShowChar(uint8_t x, uint8_t y, char Char,uint8_t FrontSize);
void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize);
void OLED_ShowImage(int16_t x, int16_t y, uint8_t Width, uint8_t Height, const uint8_t *Image);
void OLED_ShowNum(int16_t x, int16_t y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void OLED_ShowSignedNum(int16_t x, int16_t y, int32_t Number, uint8_t Length, uint8_t FontSize);
void OLED_ShowHexNum(int16_t x, int16_t y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void OLED_ShowBinNum(int16_t x, int16_t y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void OLED_ShowFloatNum(int16_t x, int16_t y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize);
void OLED_Printf(int16_t x, int16_t y, uint8_t FontSize, char *format, ...);


void OLED_DrawPoint(uint8_t x, uint8_t y);
uint8_t OLED_GetPoint(uint8_t x, uint8_t y);

void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1);
void OLED_DrawRectangle(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled);
void OLED_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled);
void OLED_DrawCircle(int16_t X, int16_t Y, uint8_t Radius, uint8_t IsFilled);
void OLED_DrawEllipse(int16_t X, int16_t Y, uint8_t A, uint8_t B, uint8_t IsFilled);
void OLED_DrawArc(int16_t X, int16_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled);

#endif