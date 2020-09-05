#ifndef __PLATFORM_BUS_H
#define __PLATFORM_BUS_H

#include "platform_kobject.h"
#include <stdio.h>

#define __PLATFORM_DEBUG

#ifdef __PLATFORM_DEBUG
#define platform_debug(fmt, args...)	printf("platform info>: " fmt, ##args)
#define platform_error(fmt, args...)	printf("platform error>: <%s>, <%d>, <%s> " fmt, __FILE__, __LINE__, __FUNCTION__, ##args)

#else
#define platform_debug(fmt, args...)
#define platform_error(fmt, args...)
#endif

extern int init_flag;

void platform_bus_probe(	const char 	*compatible, 			// STORE SET CON
				enum 		plat_driver_direct drv_direct, 	// SET-out W-in
				enum plat_driver_type drv_type,			//H L A
				int 		time_range[2],
				int 		match_point_num,		//the num of con related to this con s-w w-s
				char 		**match_point_compatibles,	//which con related to this con
				int 		extra_val
			);
void platform_bus_match_list(int event_list_id_1, int event_list_id_2);
#endif
