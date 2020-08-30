#include "delay.h"
#include "bsp_spi_lcd.h"

/**
 * @brief  SPI_lcd初始化
 * @param  无
 * @retval 无
 */
static void lcd_gpio_init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能 lcd_SPI 及GPIO 时钟 */
	/*!< SPI_lcd_SPI_CS_GPIO, SPI_lcd_SPI_MOSI_GPIO,
	     SPI_lcd_SPI_MISO_GPIO,SPI_lcd_SPI_SCK_GPIO 时钟使能 */
	RCC_AHB1PeriphClockCmd (LCD_SPI_SCK_GPIO_RCC | LCD_SPI_MISO_GPIO_RCC | LCD_SPI_MOSI_GPIO_RCC | LCD_CS_GPIO_RCC, ENABLE);

	/*!< SPI_lcd_SPI 时钟使能 */
	LCD_SPI_RCC_INIT(LCD_SPI_RCC, ENABLE);

	//设置引脚复用
	GPIO_PinAFConfig(LCD_SPI_SCK_GPIO_PORT, LCD_SPI_SCK_PINSOURCE, LCD_SPI_SCK_AF);
	GPIO_PinAFConfig(LCD_SPI_MISO_GPIO_PORT, LCD_SPI_MISO_PINSOURCE, LCD_SPI_MISO_AF);
	GPIO_PinAFConfig(LCD_SPI_MOSI_GPIO_PORT, LCD_SPI_MOSI_PINSOURCE, LCD_SPI_MOSI_AF);

	/*!< 配置 SPI_lcd_SPI 引脚: SCK */
	GPIO_InitStructure.GPIO_Pin = LCD_SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(LCD_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

	/*!< 配置 SPI_lcd_SPI 引脚: MISO */
	GPIO_InitStructure.GPIO_Pin = LCD_SPI_MISO_PIN;
	GPIO_Init(LCD_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

	/*!< 配置 SPI_lcd_SPI 引脚: MOSI */
	GPIO_InitStructure.GPIO_Pin = LCD_SPI_MOSI_PIN;
	GPIO_Init(LCD_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

	/*!< 配置 SPI_lcd_SPI 引脚: CS */
	GPIO_InitStructure.GPIO_Pin = LCD_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(LCD_CS_GPIO_PORT, &GPIO_InitStructure);

	/* 停止信号 lcd: CS引脚高电平*/
	SPI_LCD_CS_HIGH();

	/* lcd_SPI 模式配置 */
	// lcd芯片 支持SPI模式0及模式3，据此设置CPOL CPHA
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

	/* 使能 lcd_SPI  */
	SPI_Cmd(LCD_SPI, ENABLE);
	
	
	/*配置剩余引脚*/
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

	/* 等待发送缓冲区为空，TXE事件 */
	while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET);

	/* 写入数据寄存器，把要写入的数据写入发送缓冲区 */
	SPI_I2S_SendData(LCD_SPI, byte);

	/* 等待接收缓冲区非空，RXNE事件 */
	while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_RXNE) == RESET);

	/* 读取数据寄存器，获取接收缓冲区数据 */
	return SPI_I2S_ReceiveData(LCD_SPI);
}

//向液晶屏写一个8位指令
void lcd_write_addr(u8 addr)
{
	SPI_LCD_CS_LOW();
	SPI_LCD_RS_LOW();
	lcd_spi_send_byte(addr);
	SPI_LCD_CS_HIGH();
}

//向液晶屏写一个8位数据
static void lcd_write_data_8bit(u8 data)
{
	SPI_LCD_CS_LOW();
	SPI_LCD_RS_HIGH();
	lcd_spi_send_byte(data);
	SPI_LCD_CS_HIGH();
}
//向液晶屏写一个16位数据
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
函数名：LCD_Set_Region
功能：设置lcd显示区域，在此区域写点数据自动换行
入口参数：xy起点和终点
返回值：无
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
函数名：LCD_Set_XY
功能：设置lcd显示起始点
入口参数：xy坐标
返回值：无
*************************************************/
void lcd_set_xy(u16 x, u16 y)
{
  	lcd_set_region(x,y,x,y);
}

	
/*************************************************
函数名：LCD_DrawPoint
功能：画一个点
入口参数：无
返回值：无
*************************************************/
void lcd_draw_point(u16 x, u16 y, u16 data)
{
	lcd_set_region(x, y, x + 1, y + 1);
	lcd_write_data_16bit(data);

}    

/*************************************************
函数名：Lcd_Clear
功能：全屏清屏函数
入口参数：填充颜色COLOR
返回值：无
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


