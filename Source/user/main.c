#include "platform_bus.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"

char *s;
char match_points[2][20] = {"CON1", "CON2"};
enum plat_driver_type match_point_type[2] = {trigger_high, trigger_low};

const plat_device_entity_t new_device = {
	.dev_compatible = "COM1",
	.dev_id = 0,
	.dev_board_attr = {
		.GPIO = GPIOA,
		.GPIO_Pin = 1
	},
	.dev_entry = NULL
};

plat_driver_entity_t *new_driver;

int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); 
	delay_init(72);
	uart_init(115200);
	platform_device_construct_list(&device_list, 0);
	platform_event_construct_list(INIT_EVENT_LIST_ID);
	platform_event_construct_list(IN_EVENT_LIST_ID);
	platform_event_construct_list(OUT_EVENT_LIST_ID);
	s = platform_device_generate_emu_configuration();
	platform_device_parse_configuration(s);

	new_driver = platform_driver_create("CON1",drv_in, trigger_high, 1, 2, (const char **)match_points);
	platform_bus_match_devdrv(&device_list, new_driver, IN_EVENT_LIST_ID);
	platform_device_print_list(&device_list);
	
	platform_event_print_list(IN_EVENT_LIST_ID);
	while(1) {
		
	}
}
