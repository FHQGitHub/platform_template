#include "delay.h"
#include "bsp_spi_flash.h"

uint16_t SPI_FLASH_TYPE = W25Q128;

static void spi_flash_wait_busy(void);

static uint8_t SPI_FLASH_BUFFER[4096];


/**
 * 4Kbytes为一个Sector
 * 16个扇区为1个Block
 * W25Q128
 * 容量为16M字节,共有128个Block,4096个Sector
 */

/**
 * @brief  使用SPI发送一个字节的数据
 * @param  byte：要发送的数据
 * @retval 返回接收到的数据
 */
uint8_t spi_flash_send_byte(uint8_t byte)
{

	/* 等待发送缓冲区为空，TXE事件 */
	while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);

	/* 写入数据寄存器，把要写入的数据写入发送缓冲区 */
	SPI_I2S_SendData(FLASH_SPI, byte);

	/* 等待接收缓冲区非空，RXNE事件 */
	while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

	/* 读取数据寄存器，获取接收缓冲区数据 */
	return SPI_I2S_ReceiveData(FLASH_SPI);
}

/**
 * @brief  SPI_FLASH初始化
 * @param  无
 * @retval 无
 */
void spi_flash_init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能 FLASH_SPI 及GPIO 时钟 */
	/*!< SPI_FLASH_SPI_CS_GPIO, SPI_FLASH_SPI_MOSI_GPIO,
	     SPI_FLASH_SPI_MISO_GPIO,SPI_FLASH_SPI_SCK_GPIO 时钟使能 */
	RCC_AHB1PeriphClockCmd (FLASH_SPI_SCK_GPIO_CLK | FLASH_SPI_MISO_GPIO_CLK | FLASH_SPI_MOSI_GPIO_CLK | FLASH_CS_GPIO_CLK, ENABLE);

	/*!< SPI_FLASH_SPI 时钟使能 */
	FLASH_SPI_RCC_INIT(FLASH_SPI_RCC, ENABLE);

	//设置引脚复用
	GPIO_PinAFConfig(FLASH_SPI_SCK_GPIO_PORT, FLASH_SPI_SCK_PINSOURCE, FLASH_SPI_SCK_AF);
	GPIO_PinAFConfig(FLASH_SPI_MISO_GPIO_PORT, FLASH_SPI_MISO_PINSOURCE, FLASH_SPI_MISO_AF);
	GPIO_PinAFConfig(FLASH_SPI_MOSI_GPIO_PORT, FLASH_SPI_MOSI_PINSOURCE, FLASH_SPI_MOSI_AF);

	/*!< 配置 SPI_FLASH_SPI 引脚: SCK */
	GPIO_InitStructure.GPIO_Pin = FLASH_SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(FLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

	/*!< 配置 SPI_FLASH_SPI 引脚: MISO */
	GPIO_InitStructure.GPIO_Pin = FLASH_SPI_MISO_PIN;
	GPIO_Init(FLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

	/*!< 配置 SPI_FLASH_SPI 引脚: MOSI */
	GPIO_InitStructure.GPIO_Pin = FLASH_SPI_MOSI_PIN;
	GPIO_Init(FLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

	/*!< 配置 SPI_FLASH_SPI 引脚: CS */
	GPIO_InitStructure.GPIO_Pin = FLASH_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(FLASH_CS_GPIO_PORT, &GPIO_InitStructure);

	/* 停止信号 FLASH: CS引脚高电平*/
	SPI_FLASH_CS_HIGH();

	/* FLASH_SPI 模式配置 */
	// FLASH芯片 支持SPI模式0及模式3，据此设置CPOL CPHA
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(FLASH_SPI, &SPI_InitStructure);

	/* 使能 FLASH_SPI  */
	SPI_Cmd(FLASH_SPI, ENABLE);

}

/**
 * 读取W25QXX的状态寄存器
 * BIT7  6   5   4   3   2   1   0
 * SPR   RV  TB BP2 BP1 BP0 WEL BUSY
 * SPR:默认0,状态寄存器保护位,配合WP使用
 * TB,BP2,BP1,BP0:FLASH区域写保护设置
 * WEL:写使能锁定
 * BUSY:忙标记位(1：忙;	0：空闲)
 * 默认:0x00
 */
uint8_t spi_flash_read_sr(void)
{
	uint8_t byte = 0;
	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(W25X_ReadStatusReg);
	byte = spi_flash_send_byte(0xFF);
	SPI_FLASH_CS_HIGH();
	return byte;
}

/**
 * 写W25QXX状态寄存器
 * 只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
 */
void spi_flash_write_sr(uint8_t sr)
{
	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(W25X_WriteStatusReg);
	spi_flash_send_byte(sr);
	SPI_FLASH_CS_HIGH();
}
/**
 * W25QXX写使能
 * 将WEL置位
 */
void spi_flash_write_enable(void)
{
	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(W25X_WriteEnable);
	SPI_FLASH_CS_HIGH();
}
/**
 * W25QXX写禁止
 * 将WEL清零
 */
void spi_flash_write_disable(void)
{
	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(W25X_WriteDisable);
	SPI_FLASH_CS_HIGH();
}
/**
 * 读取芯片ID
 * 返回值如下:
 * 0XEF13,表示芯片型号为W25Q80
 * 0XEF14,表示芯片型号为W25Q16
 * 0XEF15,表示芯片型号为W25Q32
 * 0XEF16,表示芯片型号为W25Q64
 * 0XEF17,表示芯片型号为W25Q128
 */
uint16_t spi_flash_read_id(void)
{
	uint16_t Temp = 0;
	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(0x90);
	spi_flash_send_byte(0x00);
	spi_flash_send_byte(0x00);
	spi_flash_send_byte(0x00);
	Temp |= spi_flash_send_byte(0xFF) << 8;
	Temp |= spi_flash_send_byte(0xFF);
	SPI_FLASH_CS_HIGH();
	return Temp;
}
/**
 * 读取SPI FLASH
 * 在指定地址开始读取指定长度的数据
 * pBuffer:数据存储区
 * ReadAddr:开始读取的地址(24bit)
 * NumByteToRead:要读取的字节数(最大65535)
 */
void spi_flash_read_buffer(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	uint16_t i;

	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(W25X_ReadData);
	spi_flash_send_byte((uint8_t)((ReadAddr) >> 16));
	spi_flash_send_byte((uint8_t)((ReadAddr) >> 8));
	spi_flash_send_byte((uint8_t)ReadAddr);
	for(i = 0; i < NumByteToRead; i++) {
		pBuffer[i] = spi_flash_send_byte(0xFF);
		//printf("%d ",pBuffer[i]);
	}
	SPI_FLASH_CS_HIGH();

}
/**
 * SPI在一页(0~65535)内写入少于256个字节的数据
 * 在指定地址开始写入最大256字节的数据
 * pBuffer:数据存储区
 * WriteAddr:开始写入的地址(24bit)
 *NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!
 */
void spi_flash_write_page(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint16_t i;
	spi_flash_write_enable();
	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(W25X_PageProgram);
	spi_flash_send_byte((uint8_t)((WriteAddr) >> 16));
	spi_flash_send_byte((uint8_t)((WriteAddr) >> 8));
	spi_flash_send_byte((uint8_t)WriteAddr);
	for(i = 0; i < NumByteToWrite; i++)spi_flash_send_byte(pBuffer[i]);
	SPI_FLASH_CS_HIGH();
	spi_flash_wait_busy();
}
/**
 * 无检验写SPI FLASH
 * 必须确保所写的地址范围内的数据全部为0xFF,否则在非0xFF处写入的数据将失败!
 * 具有自动换页功能
 * 在指定地址开始写入指定长度的数据,但是要确保地址不越界!
 * pBuffer:数据存储区
 * WriteAddr:开始写入的地址(24bit)
 * NumByteToWrite:要写入的字节数(最大65535)
 */
void spi_flash_write_nocheck(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint16_t page_remain;
	page_remain = 256 - WriteAddr % 256;
	if(NumByteToWrite <= page_remain)
		page_remain = NumByteToWrite;
	while(1) {
		spi_flash_write_page(pBuffer, WriteAddr, page_remain);
		if(NumByteToWrite == page_remain)break;
		else {
			pBuffer += page_remain;
			WriteAddr += page_remain;

			NumByteToWrite -= page_remain;
			if(NumByteToWrite > 256)
				page_remain = 256;
			else 
				page_remain = NumByteToWrite;
		}
	};
}
/**
 * 写SPI FLASH
 * 在指定地址开始写入指定长度的数据
 * 该函数带擦除操作!
 * pBuffer:数据存储区
 * WriteAddr:开始写入的地址(24bit)
 * NumByteToWrite:要写入的字节数(最大65535)
*/

void spi_flash_write(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;
	uint16_t i;
	uint8_t *SPI_FLASH_BUF = SPI_FLASH_BUFFER;

	secpos = WriteAddr / 4096;
	secoff = WriteAddr % 4096;
	secremain = 4096 - secoff;
	if(NumByteToWrite <= secremain)secremain = NumByteToWrite;

	while(1) {
		spi_flash_read_buffer(SPI_FLASH_BUF, secpos * 4096, 4096);
		for(i = 0; i < secremain; i++) {
			if(SPI_FLASH_BUF[secoff + i] != 0xFF)
				break;
		}
		if(i < secremain) {
			spi_flash_erase_sector(secpos);
			for(i = 0; i < secremain; i++)
				SPI_FLASH_BUF[i + secoff] = pBuffer[i];
			spi_flash_write_nocheck(SPI_FLASH_BUF, secpos * 4096, 4096);

		} 
		else 
			spi_flash_write_nocheck(pBuffer, WriteAddr, secremain);
		if(NumByteToWrite == secremain)
			break;
		else {
			secpos++;
			secoff = 0;
			pBuffer += secremain;
			WriteAddr += secremain;
			NumByteToWrite -= secremain;
			if(NumByteToWrite > 4096)secremain = 4096;
			else secremain = NumByteToWrite;
		}
	}

}
/**
 * 擦除整个芯片
 * 等待时间超长...
 */
void spi_flash_erase_chip(void)
{
	spi_flash_write_enable();
	spi_flash_wait_busy();
	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(W25X_ChipErase);
	SPI_FLASH_CS_HIGH();
	spi_flash_wait_busy();
}
/**
 * 擦除一个扇区
 * Dst_Addr:扇区地址 根据实际容量设置
 * 擦除一个扇区的最少时间:150ms
 */
void spi_flash_erase_sector(uint32_t SectorAddr)
{
	SectorAddr *= 4096;
	spi_flash_write_enable();
	spi_flash_wait_busy();
	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(W25X_SectorErase);
	spi_flash_send_byte((uint8_t)((SectorAddr) >> 16));
	spi_flash_send_byte((uint8_t)((SectorAddr) >> 8));
	spi_flash_send_byte((uint8_t)SectorAddr);
	SPI_FLASH_CS_HIGH();
	spi_flash_wait_busy();
}

/* 进入掉电模式 */
void spi_flash_power_down(void)
{
	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(W25X_PowerDown);
	SPI_FLASH_CS_HIGH();
	delay_us(3);
}

/* 唤醒 */
void spi_flash_wake_up(void)
{
	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(W25X_ReleasePowerDown);
	SPI_FLASH_CS_HIGH();
	delay_us(3);
}

/* 等待空闲 */
static void spi_flash_wait_busy(void)
{
	while((spi_flash_read_sr() & 0x01) == 0x01);
}

