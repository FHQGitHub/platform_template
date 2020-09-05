#include "platform_driver.h"
#include "platform_bus.h"
#include "drv_delay.h"

__MUST_FREE plat_driver_entity_t *platform_driver_create(	const char 	*compatible, 			
								enum 		plat_driver_direct drv_direct, 	
								enum plat_driver_type drv_type,			
								int 		initial_val,
								int 		match_point_num,		
								char 		**match_point_compatibles
							)
{
	DRIVER_CREATE_UTILS(new_entity, plat_driver_entity_t);
	if(!ASSERT_UTILS(new_entity))
		return NULL;
	
	util_memset(new_entity, 0, sizeof(plat_driver_entity_t));
	
	strcpy(new_entity->drv_compatible, compatible);
	new_entity->kernel.drv_direct = drv_direct;
	new_entity->kernel.drv_type = drv_type;
	new_entity->kernel.initial_val = initial_val;
	new_entity->match_point_num = match_point_num;
	memcpy(new_entity->match_point_compatibles, match_point_compatibles, match_point_num * 20);
	
	return new_entity;
}
