#ifndef __PLATFORM_BUS_H
#define __PLATFORM_BUS_H

#include "platform_event.h"
#include <stdio.h>

#define platform_debug(fmt, args...)	printf("platform>: " fmt, ##args)

plat_event_entity_t *platform_bus_match_devdrv(const plat_device_list_t *device_list, const plat_driver_entity_t *driver_entity, plat_event_list_t *event_list);

#endif
