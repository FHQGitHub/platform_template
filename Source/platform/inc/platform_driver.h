#ifndef __PLATFORM_DRIVER_H
#define __PLATFORM_DRIVER_H

#include "list_utils.h"
#include "stm32f10x.h"

#define DRIVER_CREATE_UTILS(name, type)    type *name = util_alloc(1, type)

enum plat_driver_direct {
	drv_in = 0,
	drv_out
};

enum plat_driver_type {
	drv_trigger_high = 0,
	drv_trigger_low,
	drv_trigger_flip,
	drv_equals_high,
	drv_equals_low,
	drv_analog,
	drv_null
};

typedef struct {
	enum plat_driver_direct drv_direct;   // in out
	enum plat_driver_type drv_type;
	int initial_val;
}driver_kernel_t;

typedef struct {
	char drv_compatible[20];  //store name £ºconx 
	driver_kernel_t kernel;
	int match_point_num;
	char match_point_compatibles[10][20];
}plat_driver_entity_t;

__MUST_FREE plat_driver_entity_t *platform_driver_create(	const char 	*compatible, 			// STORE SET CON
								enum 		plat_driver_direct drv_direct, 	// SET-out W-in
								enum plat_driver_type drv_type,			//H L A
								int 		initial_val,
								int 		match_point_num,		//the num of con related to this con s-w w-s
								char 		**match_point_compatibles	//which con related to this con
							);
								
#endif
