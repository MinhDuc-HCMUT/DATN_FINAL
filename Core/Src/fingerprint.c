#include "stm32f1xx_hal.h"
#include "stdint.h"
#include "stdio.h"
#include "fingerprint.h"
/*****************************************************************************/
uint8_t pID;
extern UART_HandleTypeDef huart1;
/*****************************************************************************/
void USART_SendByte (uint8_t	byte)
{
	HAL_UART_Transmit(&huart1,&byte,1,500);
}
uint8_t receive_finger(uint8_t len)
{
	uint8_t p,D[13];
	while((HAL_UART_Receive(&huart1,D,len,1000))==HAL_OK);
	//HAL_UART_Receive(&huart1,D,len,500);
	p=D[len-3];
	return p;
}
uint8_t receive_finger_match(uint8_t len)
{
	uint8_t p,D[15];
//	while((HAL_UART_Receive(&huart1,D,len,500))==HAL_OK);
	HAL_UART_Receive(&huart1,D,len,1000);
	p=D[len-5];
	return p;
}
uint8_t receive_finger_search(uint8_t len)
{
	uint8_t p,D[17];
//	while((HAL_UART_Receive(&huart1,D,len,500))==HAL_OK);
	HAL_UART_Receive(&huart1,D,len,200);
	p=D[len-7];
	pID = D[11];

	return p;
}
int collect_finger(void)
{
   USART_SendByte(0xEF);USART_SendByte(0x01);
   USART_SendByte(0xFF);USART_SendByte(0xFF);USART_SendByte(0xFF);USART_SendByte(0xFF);
   USART_SendByte(0x01);
   USART_SendByte(0x00);USART_SendByte(0x03);
   USART_SendByte(0x01);
   USART_SendByte(0x00);USART_SendByte(0x05);
   return receive_finger(12);
}
int img2tz(uint8_t local)
{//ghi du lieu van tay vao bo nho dem local(local co the la: 0x01 vung 1, 0x02 vung 2)
  int  sum = 0x00;
   sum = local + 0x07;
   USART_SendByte(0xEF);USART_SendByte(0x01);
   USART_SendByte(0xFF);USART_SendByte(0xFF);USART_SendByte(0xFF);USART_SendByte(0xFF);
   USART_SendByte(0x01);
   USART_SendByte(0x00);USART_SendByte(0x04);
   USART_SendByte(0x02);
   USART_SendByte(local);
   USART_SendByte(0x00);USART_SendByte(sum);
   return receive_finger(12);
}
int match(void)
{//so s�nh 2 bo dem ve trung khop van tay

   USART_SendByte(0xEF);USART_SendByte(0x01);
   USART_SendByte(0xFF);USART_SendByte(0xFF);USART_SendByte(0xFF);USART_SendByte(0xFF);
   USART_SendByte(0x01);
   USART_SendByte(0x00);USART_SendByte(0x03);
   USART_SendByte(0x03);
   USART_SendByte(0x00);USART_SendByte(0x07);
   return receive_finger_match(14);
}
int regmodel(void)
{//tao ma van tay chuan tu 2 bo dem
   USART_SendByte(0xEF);USART_SendByte(0x01);
   USART_SendByte(0XFF);USART_SendByte(0XFF);USART_SendByte(0XFF);USART_SendByte(0XFF);
   USART_SendByte(0x01);
   USART_SendByte(0x00);USART_SendByte(0x03);
   USART_SendByte(0x05);
   USART_SendByte(0x00);USART_SendByte(0x09);
   return receive_finger(12);
   //if (tmp==0x00){
   //LCD_Clear(YELLOW);LCD_ShowString(80,80,(unsigned char*)"da lay mau",0x001F ,YELLOW);DELAY_MS(2500000);}
}
int store(uint8_t ID)
{// luu ma van tay chuan vao flash
//	int D[20];
   uint8_t sum1;
 //  for(i=0;i<20;i++) D[i]=0xDD;
   sum1= 0x0E + ID;
   USART_SendByte(0xEF);USART_SendByte(0x01);
   USART_SendByte(0XFF);USART_SendByte(0XFF);USART_SendByte(0XFF);USART_SendByte(0XFF);
   USART_SendByte(0x01);
   USART_SendByte(0x00);USART_SendByte(0x06);
   USART_SendByte(0x06);
   USART_SendByte(0x01);
   USART_SendByte(0x00);USART_SendByte(ID);
   USART_SendByte(0x00);USART_SendByte(sum1);
   return receive_finger(12);
//    if (tmp==0x00)
//    {
//      LCD_Clear(YELLOW);LCD_ShowString(80,80,(unsigned char*)
//       "da luu",0x001F ,YELLOW);
//      DELAY_MS(500);
//    }
}
int search(void)
{//l�i ma van tay chua tu flash ra de so sanh voi van tay vua nhan tren bo dem
   USART_SendByte(0xEF);USART_SendByte(0x01);
   USART_SendByte(0XFF);USART_SendByte(0XFF);USART_SendByte(0XFF);USART_SendByte(0XFF);
	// kiem tra check sum tu day
   USART_SendByte(0x01);
   USART_SendByte(0x00);USART_SendByte(0x08);
   USART_SendByte(0x04);
   USART_SendByte(0x01);
   USART_SendByte(0x00);USART_SendByte(0x00);// dia chi bat dau
////   USART_SendByte(0x00);USART_SendByte(0xFF);
	USART_SendByte(0x00);USART_SendByte(0xff);// dia chi ket thuc
	//ket thuc kt ch�chum
//  USART_SendByte(0x00);USART_SendByte(0x0F);// ma check sum dc tinh
		USART_SendByte(0x01);USART_SendByte(0x0D);// ma check sum dc tinh
   return receive_finger_search(16);
}
int search1(void)
{//l�i ma van tay chua tu flash ra de so sanh voi van tay vua nhan tren bo dem
   USART_SendByte(0xEF);USART_SendByte(0x01);
   USART_SendByte(0XFF);USART_SendByte(0XFF);USART_SendByte(0XFF);USART_SendByte(0XFF);
	// kiem tra check sum tu day
   USART_SendByte(0x01);
   USART_SendByte(0x00);USART_SendByte(0x08);
   USART_SendByte(0x04);
   USART_SendByte(0x01);
   USART_SendByte(0x00);USART_SendByte(0x00);// dia chi bat dau
////   USART_SendByte(0x00);USART_SendByte(0xFF);
	USART_SendByte(0x00);USART_SendByte(0x01);// dia chi ket thuc
	//ket thuc kt ch�chum
  USART_SendByte(0x00);USART_SendByte(0x0F);// ma check sum dc tinh
//		USART_SendByte(0x01);USART_SendByte(0x0D);// ma check sum dc tinh
   return receive_finger_search(16);

}
int empty(void)
{
//   tmp=0xFF;
//	int D[20];
 //  for(i=0;i<20;i++) D[i]=0xDD;
   USART_SendByte(0xEF);USART_SendByte(0x01);
   USART_SendByte(0xFF);USART_SendByte(0xFF);USART_SendByte(0xFF);USART_SendByte(0xFF);
   USART_SendByte(0x01);
   USART_SendByte(0x00);USART_SendByte(0x03);
   USART_SendByte(0x0D);
   USART_SendByte(0x00);USART_SendByte(0x11);
   return receive_finger(12);

}
int del(uint8_t id)
{
	uint8_t sum1;
    sum1 = 0x15 + id;
    USART_SendByte(0xEF);USART_SendByte(0x01);
    USART_SendByte(0xFF);USART_SendByte(0xFF);USART_SendByte(0xFF);USART_SendByte(0xFF);
    USART_SendByte(0x01);
    USART_SendByte(0x00);USART_SendByte(0x07);
    USART_SendByte(0x0C);
    USART_SendByte(0x00);USART_SendByte(id);
    USART_SendByte(0x00);USART_SendByte(0x01);
    USART_SendByte(0x00);USART_SendByte(sum1);
    return receive_finger(12);
}

uint8_t delete_finger(uint8_t id)
{
    return del(id);
}

uint8_t delete_all_fingers()
{
    uint8_t buffer[14];
    buffer[0] = 0xEF; // Start code
    buffer[1] = 0x01; // Start code
    buffer[2] = 0xFF; // Address
    buffer[3] = 0xFF; // Address
    buffer[4] = 0xFF; // Address
    buffer[5] = 0xFF; // Address
    buffer[6] = 0x01; // Command packet
    buffer[7] = 0x00; // Packet length
    buffer[8] = 0x07; // Packet length
    buffer[9] = 0x0D; // Empty command
    buffer[10] = 0x00; // Start ID
    buffer[11] = 0x00; // Number of templates to delete (0 for all)

    uint16_t checksum = 0;
    for (int i = 6; i < 12; i++)
    {
        checksum += buffer[i];
    }
    buffer[12] = (checksum >> 8) & 0xFF; // Checksum high byte
    buffer[13] = checksum & 0xFF;        // Checksum low byte

    HAL_UART_Transmit(&huart1, buffer, 14, HAL_MAX_DELAY);

    uint8_t response[12];
    HAL_StatusTypeDef status = HAL_UART_Receive(&huart1, response, 12, 5000); // Increase timeout

    if (status == HAL_OK)
    {
        if (response[9] == 0x00)
        {
            return 0x00; // Success
        }
        else
        {
            return response[9]; // Error code from the module
        }
    }
    else
    {
        return 0xFF; // UART receive error
    }
}
