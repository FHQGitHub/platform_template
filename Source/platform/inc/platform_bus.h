#ifndef __PLATFORM_BUS_H
#define __PLATFORM_BUS_H

#include "platform_event.h"
#include <stdio.h>

#define platform_debug(fmt, args...)	printf("platform>: " fmt, ##args)

void platform_bus_probe(	const char 	*compatible, 			// STORE SET CON
				enum 		plat_driver_direct drv_direct, 	// SET-out W-in
				enum plat_driver_type drv_type,			//H L A
				int 		initial_val,
				int 		time_range[2],
				int 		match_point_num,		//the num of con related to this con s-w w-s
				const char 	**match_point_compatibles	//which con related to this con
			  );
#endif
