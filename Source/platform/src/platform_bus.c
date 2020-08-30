#include "platform_bus.h"
#include "platform_event.h"

static plat_event_entity_t *platform_bus_generate_event(const plat_device_entity_t *device_entity, const plat_driver_entity_t *driver_entity)
{
	plat_event_entity_t *new_event = NULL;
	if(!ASSERT_UTILS(device_entity) || !ASSERT_UTILS(driver_entity))
		return NULL;
	
	new_event = platform_event_create();
	if(NULL == new_event)
		return NULL;
	
	strcpy(new_event->ev_compatible, device_entity->dev_compatible);
	new_event->ev_board_attr.GPIO = device_entity->dev_board_attr.GPIO;
	new_event->ev_board_attr.GPIO_Pin = device_entity->dev_board_attr.GPIO_Pin;
	new_event->ev_direct = driver_entity->drv_direct;
	
	return new_event;
}

plat_event_entity_t *platform_bus_match_devdrv(const plat_device_list_t *device_list, const plat_driver_entity_t *driver_entity, plat_event_list_t *event_list)
{
	struct list_head *t = NULL;
        plat_device_entity_t *p = NULL;
	plat_event_entity_t *new_event = NULL;
	plat_event_entity_t *match_event = NULL;
	
	list_for_each(t, device_list->device_head) {
		p = list_entry(t, plat_device_entity_t);
		if(!strcmp(p->dev_compatible, driver_entity->drv_compatible)) {
			match_event = platform_event_get_entity_v_compatible(event_list, p->dev_compatible);
			if(NULL == match_event) {
				new_event = platform_bus_generate_event(p, driver_entity);
				if(NULL != new_event)
					platform_event_register(&in_event_list, new_event);
				match_event = new_event;
			}
			else {
				
			}
			return match_event;
		}
	}
	
	platform_debug("can't match driver with compatible <%s>", driver_entity->drv_compatible);
	return NULL;
}
