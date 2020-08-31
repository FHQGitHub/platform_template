#include "platform_driver.h"
#include "platform_bus.h"

static void platform_drv_gpio_init(GPIO_TypeDef *GPIO, uint16_t GPIO_Pin, enum plat_driver_direct gpio_direct, enum plat_driver_type gpio_type);
static uint16_t platform_drv_gpio_read(GPIO_TypeDef *GPIO, uint16_t GPIO_Pin, enum plat_driver_type gpio_type);
static void platform_drv_gpio_write(GPIO_TypeDef *GPIO, uint16_t GPIO_Pin, enum plat_driver_type gpio_type, uint16_t val);

device_operations_t dops = {
	.init = platform_drv_gpio_init,
	.read = platform_drv_gpio_read,
	.write = platform_drv_gpio_write
};

__MUST_FREE plat_driver_entity_t *platform_driver_create(	const char 	*compatible, 			// STORE SET CON
								enum 		plat_driver_direct drv_direct, 	// SET-out W-in
								enum plat_driver_type drv_type,			//H L A
								int 		initial_val,
								int 		match_point_num,		//the num of con related to this con s-w w-s
								const char 	**match_point_compatibles	//which con related to this con
							)
{
	DRIVER_CREATE_UTILS(new_entity, plat_driver_entity_t);
	if(!ASSERT_UTILS(new_entity))
		return NULL;
	
	util_memset(new_entity, 0, sizeof(plat_driver_entity_t));
	
	strcpy(new_entity->drv_compatible, compatible);
	new_entity->drv_direct = drv_direct;
	new_entity->drv_type = drv_type;
	new_entity->initial_val = initial_val;
	new_entity->match_point_num = match_point_num;
	memcpy(new_entity->match_point_compatibles, match_point_compatibles, match_point_num * 20);
	
	return new_entity;
}


static void platform_drv_gpio_init(GPIO_TypeDef *GPIO, uint16_t GPIO_Pin, enum plat_driver_direct gpio_direct, enum plat_driver_type gpio_type)
{
	
}

static uint16_t platform_drv_gpio_read(GPIO_TypeDef *GPIO, uint16_t GPIO_Pin, enum plat_driver_type gpio_type)
{
	return 0; //to return a digital value<0:1> or an analog value<0:4096>
}

static void platform_drv_gpio_write(GPIO_TypeDef *GPIO, uint16_t GPIO_Pin, enum plat_driver_type gpio_type, uint16_t val)
{
	
}

