#ifndef __PLATFORM_DRIVER_H
#define __PLATFORM_DRIVER_H

#include "list_utils.h"
#include "stm32f4xx.h"
#include "platform_device.h"

#define DRIVER_CREATE_UTILS(name, type)    type *name = util_alloc(1, type)

enum plat_driver_direct {
	drv_in = 0,
	drv_out
};

enum plat_driver_type {
	trigger_high = 0,
	trigger_low,
	analog
};

typedef struct {
	char drv_compatible[20];  //store name £ºconx 
	enum plat_driver_direct drv_direct;   // in out
	enum plat_driver_type drv_type;
	int match_points_num;
	char match_points[10][20];
}plat_driver_entity_t;

typedef struct {
	void (*init)(GPIO_TypeDef *, uint16_t, enum plat_driver_direct, enum plat_driver_type);
	uint16_t (*read)(GPIO_TypeDef *, uint16_t, enum plat_driver_type);
	void (*write)(GPIO_TypeDef *, uint16_t, enum plat_driver_type, uint16_t);
}device_operations_t;

extern device_operations_t dops;

__MUST_FREE plat_driver_entity_t *platform_driver_create(	const char 	*compatible, 			// STORE SET CON
								enum 		plat_driver_direct drv_direct, 	// SET-out W-in
								enum plat_driver_type drv_type,			
								int 		match_points_num,
								const char 	**match_points
							);

#endif
