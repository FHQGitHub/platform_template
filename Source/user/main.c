#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "bsp_timer.h"
#include "drv_uart.h"
#include "drv_delay.h"
#include "drv_spi.h"
#include "drv_SI446x.h"
#include "platform_event.h"
#include "platform_bus.h"
#include "message_process.h"
#include "state_process.h"
#include "kshell.h"

char s[3][100] = {
			"W 'SWT1'H ,S 'LED4'H.",
//			"W 'SWT2'H ,S 'LED1'L."
			"O"
		};


int main()
{
	int i = 0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); 
	drv_delay_init();
	uart_init(115200);
	timer3_init(7199, 999);
	drv_spi_init();
	SI446x_Init();
	device_list = platform_device_construct_list(0);
	kobject_list = platform_kobject_construct_list(0);
	platform_event_construct_list(INIT_EVENT_LIST_ID);
	platform_event_construct_list(IN_EVENT_LIST_ID);
	platform_event_construct_list(OUT_EVENT_LIST_ID);
	
	TIM_Cmd(TIM3, ENABLE); 	
	while(1) {
		if(init_flag) {
			platform_event_handle_hw_status();
			platform_event_handle_list(IN_EVENT_LIST_ID);
			platform_event_handle_list(OUT_EVENT_LIST_ID);
		}
		if(USART_RX_STA & 0x8000) {
			exec((char *)USART_RX_BUF);
			memset(USART_RX_BUF, 0, 2000);
			USART_RX_STA = 0;	
		}
	}
}
