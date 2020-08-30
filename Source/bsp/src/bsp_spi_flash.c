#include "delay.h"
#include "bsp_spi_flash.h"

uint16_t SPI_FLASH_TYPE = W25Q128;

static void spi_flash_wait_busy(void);

static uint8_t SPI_FLASH_BUFFER[4096];


/**
 * 4KbytesΪһ��Sector
 * 16������Ϊ1��Block
 * W25Q128
 * ����Ϊ16M�ֽ�,����128��Block,4096��Sector
 */

/**
 * @brief  ʹ��SPI����һ���ֽڵ�����
 * @param  byte��Ҫ���͵�����
 * @retval ���ؽ��յ�������
 */
uint8_t spi_flash_send_byte(uint8_t byte)
{

	/* �ȴ����ͻ�����Ϊ�գ�TXE�¼� */
	while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);

	/* д�����ݼĴ�������Ҫд�������д�뷢�ͻ����� */
	SPI_I2S_SendData(FLASH_SPI, byte);

	/* �ȴ����ջ������ǿգ�RXNE�¼� */
	while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

	/* ��ȡ���ݼĴ�������ȡ���ջ��������� */
	return SPI_I2S_ReceiveData(FLASH_SPI);
}

/**
 * @brief  SPI_FLASH��ʼ��
 * @param  ��
 * @retval ��
 */
void spi_flash_init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ʹ�� FLASH_SPI ��GPIO ʱ�� */
	/*!< SPI_FLASH_SPI_CS_GPIO, SPI_FLASH_SPI_MOSI_GPIO,
	     SPI_FLASH_SPI_MISO_GPIO,SPI_FLASH_SPI_SCK_GPIO ʱ��ʹ�� */
	RCC_AHB1PeriphClockCmd (FLASH_SPI_SCK_GPIO_CLK | FLASH_SPI_MISO_GPIO_CLK | FLASH_SPI_MOSI_GPIO_CLK | FLASH_CS_GPIO_CLK, ENABLE);

	/*!< SPI_FLASH_SPI ʱ��ʹ�� */
	FLASH_SPI_RCC_INIT(FLASH_SPI_RCC, ENABLE);

	//�������Ÿ���
	GPIO_PinAFConfig(FLASH_SPI_SCK_GPIO_PORT, FLASH_SPI_SCK_PINSOURCE, FLASH_SPI_SCK_AF);
	GPIO_PinAFConfig(FLASH_SPI_MISO_GPIO_PORT, FLASH_SPI_MISO_PINSOURCE, FLASH_SPI_MISO_AF);
	GPIO_PinAFConfig(FLASH_SPI_MOSI_GPIO_PORT, FLASH_SPI_MOSI_PINSOURCE, FLASH_SPI_MOSI_AF);

	/*!< ���� SPI_FLASH_SPI ����: SCK */
	GPIO_InitStructure.GPIO_Pin = FLASH_SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(FLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

	/*!< ���� SPI_FLASH_SPI ����: MISO */
	GPIO_InitStructure.GPIO_Pin = FLASH_SPI_MISO_PIN;
	GPIO_Init(FLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

	/*!< ���� SPI_FLASH_SPI ����: MOSI */
	GPIO_InitStructure.GPIO_Pin = FLASH_SPI_MOSI_PIN;
	GPIO_Init(FLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

	/*!< ���� SPI_FLASH_SPI ����: CS */
	GPIO_InitStructure.GPIO_Pin = FLASH_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(FLASH_CS_GPIO_PORT, &GPIO_InitStructure);

	/* ֹͣ�ź� FLASH: CS���Ÿߵ�ƽ*/
	SPI_FLASH_CS_HIGH();

	/* FLASH_SPI ģʽ���� */
	// FLASHоƬ ֧��SPIģʽ0��ģʽ3���ݴ�����CPOL CPHA
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

	/* ʹ�� FLASH_SPI  */
	SPI_Cmd(FLASH_SPI, ENABLE);

}

/**
 * ��ȡW25QXX��״̬�Ĵ���
 * BIT7  6   5   4   3   2   1   0
 * SPR   RV  TB BP2 BP1 BP0 WEL BUSY
 * SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
 * TB,BP2,BP1,BP0:FLASH����д��������
 * WEL:дʹ������
 * BUSY:æ���λ(1��æ;	0������)
 * Ĭ��:0x00
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
 * дW25QXX״̬�Ĵ���
 * ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
 */
void spi_flash_write_sr(uint8_t sr)
{
	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(W25X_WriteStatusReg);
	spi_flash_send_byte(sr);
	SPI_FLASH_CS_HIGH();
}
/**
 * W25QXXдʹ��
 * ��WEL��λ
 */
void spi_flash_write_enable(void)
{
	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(W25X_WriteEnable);
	SPI_FLASH_CS_HIGH();
}
/**
 * W25QXXд��ֹ
 * ��WEL����
 */
void spi_flash_write_disable(void)
{
	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(W25X_WriteDisable);
	SPI_FLASH_CS_HIGH();
}
/**
 * ��ȡоƬID
 * ����ֵ����:
 * 0XEF13,��ʾоƬ�ͺ�ΪW25Q80
 * 0XEF14,��ʾоƬ�ͺ�ΪW25Q16
 * 0XEF15,��ʾоƬ�ͺ�ΪW25Q32
 * 0XEF16,��ʾоƬ�ͺ�ΪW25Q64
 * 0XEF17,��ʾоƬ�ͺ�ΪW25Q128
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
 * ��ȡSPI FLASH
 * ��ָ����ַ��ʼ��ȡָ�����ȵ�����
 * pBuffer:���ݴ洢��
 * ReadAddr:��ʼ��ȡ�ĵ�ַ(24bit)
 * NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
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
 * SPI��һҳ(0~65535)��д������256���ֽڵ�����
 * ��ָ����ַ��ʼд�����256�ֽڵ�����
 * pBuffer:���ݴ洢��
 * WriteAddr:��ʼд��ĵ�ַ(24bit)
 *NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!
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
 * �޼���дSPI FLASH
 * ����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0xFF,�����ڷ�0xFF��д������ݽ�ʧ��!
 * �����Զ���ҳ����
 * ��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
 * pBuffer:���ݴ洢��
 * WriteAddr:��ʼд��ĵ�ַ(24bit)
 * NumByteToWrite:Ҫд����ֽ���(���65535)
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
 * дSPI FLASH
 * ��ָ����ַ��ʼд��ָ�����ȵ�����
 * �ú�������������!
 * pBuffer:���ݴ洢��
 * WriteAddr:��ʼд��ĵ�ַ(24bit)
 * NumByteToWrite:Ҫд����ֽ���(���65535)
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
 * ��������оƬ
 * �ȴ�ʱ�䳬��...
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
 * ����һ������
 * Dst_Addr:������ַ ����ʵ����������
 * ����һ������������ʱ��:150ms
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

/* �������ģʽ */
void spi_flash_power_down(void)
{
	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(W25X_PowerDown);
	SPI_FLASH_CS_HIGH();
	delay_us(3);
}

/* ���� */
void spi_flash_wake_up(void)
{
	SPI_FLASH_CS_LOW();
	spi_flash_send_byte(W25X_ReleasePowerDown);
	SPI_FLASH_CS_HIGH();
	delay_us(3);
}

/* �ȴ����� */
static void spi_flash_wait_busy(void)
{
	while((spi_flash_read_sr() & 0x01) == 0x01);
}

