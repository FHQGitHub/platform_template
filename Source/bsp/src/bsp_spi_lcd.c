#include "delay.h"
#include "bsp_spi_lcd.h"

/**
 * @brief  SPI_lcd��ʼ��
 * @param  ��
 * @retval ��
 */
static void lcd_gpio_init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ʹ�� lcd_SPI ��GPIO ʱ�� */
	/*!< SPI_lcd_SPI_CS_GPIO, SPI_lcd_SPI_MOSI_GPIO,
	     SPI_lcd_SPI_MISO_GPIO,SPI_lcd_SPI_SCK_GPIO ʱ��ʹ�� */
	RCC_AHB1PeriphClockCmd (LCD_SPI_SCK_GPIO_RCC | LCD_SPI_MISO_GPIO_RCC | LCD_SPI_MOSI_GPIO_RCC | LCD_CS_GPIO_RCC, ENABLE);

	/*!< SPI_lcd_SPI ʱ��ʹ�� */
	LCD_SPI_RCC_INIT(LCD_SPI_RCC, ENABLE);

	//�������Ÿ���
	GPIO_PinAFConfig(LCD_SPI_SCK_GPIO_PORT, LCD_SPI_SCK_PINSOURCE, LCD_SPI_SCK_AF);
	GPIO_PinAFConfig(LCD_SPI_MISO_GPIO_PORT, LCD_SPI_MISO_PINSOURCE, LCD_SPI_MISO_AF);
	GPIO_PinAFConfig(LCD_SPI_MOSI_GPIO_PORT, LCD_SPI_MOSI_PINSOURCE, LCD_SPI_MOSI_AF);

	/*!< ���� SPI_lcd_SPI ����: SCK */
	GPIO_InitStructure.GPIO_Pin = LCD_SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(LCD_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

	/*!< ���� SPI_lcd_SPI ����: MISO */
	GPIO_InitStructure.GPIO_Pin = LCD_SPI_MISO_PIN;
	GPIO_Init(LCD_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

	/*!< ���� SPI_lcd_SPI ����: MOSI */
	GPIO_InitStructure.GPIO_Pin = LCD_SPI_MOSI_PIN;
	GPIO_Init(LCD_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

	/*!< ���� SPI_lcd_SPI ����: CS */
	GPIO_InitStructure.GPIO_Pin = LCD_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(LCD_CS_GPIO_PORT, &GPIO_InitStructure);

	/* ֹͣ�ź� lcd: CS���Ÿߵ�ƽ*/
	SPI_LCD_CS_HIGH();

	/* lcd_SPI ģʽ���� */
	// lcdоƬ ֧��SPIģʽ0��ģʽ3���ݴ�����CPOL CPHA
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(LCD_SPI, &SPI_InitStructure);

	/* ʹ�� lcd_SPI  */
	SPI_Cmd(LCD_SPI, ENABLE);
	
	
	/*����ʣ������*/
	RCC_AHB1PeriphClockCmd(LCD_RS_GPIO_RCC | LCD_BLK_GPIO_RCC | LCD_RST_GPIO_RCC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = LCD_RS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(LCD_RS_GPIO_PORT, &GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin = LCD_BLK_PIN;
	GPIO_Init(LCD_BLK_GPIO_PORT, &GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin = LCD_RST_PIN;
	GPIO_Init(LCD_RST_GPIO_PORT, &GPIO_InitStructure); 
	
}

static uint8_t lcd_spi_send_byte(uint8_t byte)
{

	/* �ȴ����ͻ�����Ϊ�գ�TXE�¼� */
	while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET);

	/* д�����ݼĴ�������Ҫд�������д�뷢�ͻ����� */
	SPI_I2S_SendData(LCD_SPI, byte);

	/* �ȴ����ջ������ǿգ�RXNE�¼� */
	while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_RXNE) == RESET);

	/* ��ȡ���ݼĴ�������ȡ���ջ��������� */
	return SPI_I2S_ReceiveData(LCD_SPI);
}

//��Һ����дһ��8λָ��
void lcd_write_addr(u8 addr)
{
	SPI_LCD_CS_LOW();
	SPI_LCD_RS_LOW();
	lcd_spi_send_byte(addr);
	SPI_LCD_CS_HIGH();
}

//��Һ����дһ��8λ����
static void lcd_write_data_8bit(u8 data)
{
	SPI_LCD_CS_LOW();
	SPI_LCD_RS_HIGH();
	lcd_spi_send_byte(data);
	SPI_LCD_CS_HIGH();
}
//��Һ����дһ��16λ����
void lcd_write_data_16bit(u16 data)
{
	SPI_LCD_CS_LOW();
	SPI_LCD_RS_HIGH();
	lcd_spi_send_byte(data >> 8);
	lcd_spi_send_byte(data);
	SPI_LCD_CS_HIGH();
}

static void lcd_write_reg(u8 addr, u8 data)
{
	lcd_write_addr(addr);
	lcd_write_data_8bit(data);
}

void lcd_reset()
{
	SPI_LCD_RST_LOW();
	delay_ms(100);
	SPI_LCD_RST_HIGH();
	delay_ms(50);
}

void lcd_init()
{	
	lcd_gpio_init();
	lcd_reset(); //Reset before LCD Init.

	//LCD Init For 1.44Inch LCD Panel with ST7735R.
	lcd_write_addr(0x11);//Sleep exit 
	delay_ms(120);
		
	//ST7735R Frame Rate
	lcd_write_addr(0xB1); 
	lcd_write_data_8bit(0x01); 
	lcd_write_data_8bit(0x2C); 
	lcd_write_data_8bit(0x2D); 

	lcd_write_addr(0xB2); 
	lcd_write_data_8bit(0x01); 
	lcd_write_data_8bit(0x2C); 
	lcd_write_data_8bit(0x2D); 

	lcd_write_addr(0xB3); 
	lcd_write_data_8bit(0x01); 
	lcd_write_data_8bit(0x2C); 
	lcd_write_data_8bit(0x2D); 
	lcd_write_data_8bit(0x01); 
	lcd_write_data_8bit(0x2C); 
	lcd_write_data_8bit(0x2D); 
	
	lcd_write_addr(0xB4); //Column inversion 
	lcd_write_data_8bit(0x07); 
	
	//ST7735R Power Sequence
	lcd_write_addr(0xC0); 
	lcd_write_data_8bit(0xA2); 
	lcd_write_data_8bit(0x02); 
	lcd_write_data_8bit(0x84); 
	lcd_write_addr(0xC1); 
	lcd_write_data_8bit(0xC5); 

	lcd_write_addr(0xC2); 
	lcd_write_data_8bit(0x0A); 
	lcd_write_data_8bit(0x00); 

	lcd_write_addr(0xC3); 
	lcd_write_data_8bit(0x8A); 
	lcd_write_data_8bit(0x2A); 
	lcd_write_addr(0xC4); 
	lcd_write_data_8bit(0x8A); 
	lcd_write_data_8bit(0xEE); 
	
	lcd_write_addr(0xC5); //VCOM 
	lcd_write_data_8bit(0x0E); 
	
	lcd_write_addr(0x36); //MX, MY, RGB mode 
	lcd_write_data_8bit(0xC8); 
	
	//ST7735R Gamma Sequence
	lcd_write_addr(0xe0); 
	lcd_write_data_8bit(0x0f); 
	lcd_write_data_8bit(0x1a); 
	lcd_write_data_8bit(0x0f); 
	lcd_write_data_8bit(0x18); 
	lcd_write_data_8bit(0x2f); 
	lcd_write_data_8bit(0x28); 
	lcd_write_data_8bit(0x20); 
	lcd_write_data_8bit(0x22); 
	lcd_write_data_8bit(0x1f); 
	lcd_write_data_8bit(0x1b); 
	lcd_write_data_8bit(0x23); 
	lcd_write_data_8bit(0x37); 
	lcd_write_data_8bit(0x00); 	
	lcd_write_data_8bit(0x07); 
	lcd_write_data_8bit(0x02); 
	lcd_write_data_8bit(0x10); 

	lcd_write_addr(0xe1); 
	lcd_write_data_8bit(0x0f); 
	lcd_write_data_8bit(0x1b); 
	lcd_write_data_8bit(0x0f); 
	lcd_write_data_8bit(0x17); 
	lcd_write_data_8bit(0x33); 
	lcd_write_data_8bit(0x2c); 
	lcd_write_data_8bit(0x29); 
	lcd_write_data_8bit(0x2e); 
	lcd_write_data_8bit(0x30); 
	lcd_write_data_8bit(0x30); 
	lcd_write_data_8bit(0x39); 
	lcd_write_data_8bit(0x3f); 
	lcd_write_data_8bit(0x00); 
	lcd_write_data_8bit(0x07); 
	lcd_write_data_8bit(0x03); 
	lcd_write_data_8bit(0x10);  
	
	lcd_write_addr(0x2a);
	lcd_write_data_8bit(0x00);
	lcd_write_data_8bit(0x00);
	lcd_write_data_8bit(0x00);
	lcd_write_data_8bit(0x7f);

	lcd_write_addr(0x2b);
	lcd_write_data_8bit(0x00);
	lcd_write_data_8bit(0x00);
	lcd_write_data_8bit(0x00);
	lcd_write_data_8bit(0x9f);
	
	lcd_write_addr(0xF0); //Enable test command  
	lcd_write_data_8bit(0x01); 
	lcd_write_addr(0xF6); //Disable ram power save mode 
	lcd_write_data_8bit(0x00); 
	
	lcd_write_addr(0x3A); //65k mode 
	lcd_write_data_8bit(0x05); 
	
	lcd_write_addr(0x29);//Display on	
	lcd_clear(LCD_BLUE);
	SPI_LCD_BLK_HIGH();
}


/*************************************************
��������LCD_Set_Region
���ܣ�����lcd��ʾ�����ڴ�����д�������Զ�����
��ڲ�����xy�����յ�
����ֵ����
*************************************************/
void lcd_set_region(u16 x_start, u16 y_start, u16 x_end, u16 y_end)
{		
	lcd_write_addr(0x2a);
	lcd_write_data_8bit(0x00);
	lcd_write_data_8bit(x_start + 2);
	lcd_write_data_8bit(0x00);
	lcd_write_data_8bit(x_end + 2);

	lcd_write_addr(0x2b);
	lcd_write_data_8bit(0x00);
	lcd_write_data_8bit(y_start + 3);
	lcd_write_data_8bit(0x00);
	lcd_write_data_8bit(y_end + 3);
	
	lcd_write_addr(0x2c);

}

/*************************************************
��������LCD_Set_XY
���ܣ�����lcd��ʾ��ʼ��
��ڲ�����xy����
����ֵ����
*************************************************/
void lcd_set_xy(u16 x, u16 y)
{
  	lcd_set_region(x,y,x,y);
}

	
/*************************************************
��������LCD_DrawPoint
���ܣ���һ����
��ڲ�������
����ֵ����
*************************************************/
void lcd_draw_point(u16 x, u16 y, u16 data)
{
	lcd_set_region(x, y, x + 1, y + 1);
	lcd_write_data_16bit(data);

}    

/*************************************************
��������Lcd_Clear
���ܣ�ȫ����������
��ڲ����������ɫCOLOR
����ֵ����
*************************************************/
void lcd_clear(u16 color)               
{	
	unsigned int i,m;
	lcd_set_region(0, 0, X_MAX_PIXEL-1, Y_MAX_PIXEL-1);
	lcd_write_addr(0x2C);
	for(i=0;i < X_MAX_PIXEL; i++)
		for(m = 0;m < Y_MAX_PIXEL; m++)
			lcd_write_data_16bit(color);
}


