#include "bsp_iic.h"
#include "delay.h"

void i2c_gpio_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(IIC_GPIO_RCC, ENABLE); 
	
	GPIO_InitStructure.GPIO_Pin = IIC_SCL | IIC_SDA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(GPIOC,&GPIO_InitStructure); 
	
}

void i2c_start(void)
{
	IIC_SCL_H();
	IIC_SDA_H();
	IIC_SDA_L();
	IIC_SCL_L();
}
void i2c_stop(void)
{
	IIC_SCL_L();
	IIC_SDA_L();
	IIC_SDA_H();
	IIC_SCL_H();	
}
void i2c_send_byte(u8 dat)
{
	u8 i;
	for(i=0; i<8; i++) {
		IIC_SCL_L();
		if(dat & 0x80)
			IIC_SDA_H();
		else
			IIC_SDA_L();
		dat <<= 1;
		IIC_SCL_H();
			
	}
	IIC_SCL_L();
	IIC_SCL_H();
}
void i2c_delay_ms(u16 time)
{
	for(; time; time--) {
		delay_ms(1);
	}
}
