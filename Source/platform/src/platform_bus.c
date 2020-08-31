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

plat_event_entity_t *platform_bus_match_devdrv(const plat_device_list_t *device_list, const plat_driver_entity_t *driver_entity, int event_list_id)
{
	struct list_head *t = NULL;
        plat_device_entity_t *p = NULL;
	plat_event_entity_t *new_event = NULL;
	plat_event_entity_t *match_event = NULL;
	plat_event_list_t *event_list = NULL;
	
	event_list = plat_event_get_list_v_id(event_list_id);
	if(NULL == event_list)
		return NULL;
	
	list_for_each(t, device_list->device_head) {
		p = list_entry(t, plat_device_entity_t);
		if(!strcmp(p->dev_compatible, driver_entity->drv_compatible)) {
			match_event = platform_event_get_entity_v_compatible(event_list, p->dev_compatible);
			if(NULL == match_event) {
				new_event = platform_bus_generate_event(p, driver_entity);
				if(NULL != new_event)
					platform_event_register(IN_EVENT_LIST_ID, new_event);
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

void platform_bus_probe(	const char 	*compatible, 			// STORE SET CON
				enum 		plat_driver_direct drv_direct, 	// SET-out W-in
				enum plat_driver_type drv_type,			//H L A
				int 		time_range[2],
				int 		match_point_num,		//the num of con related to this con s-w w-s
				const char 	**match_point_compatibles,	//which con related to this con
				int 		extra_val
			  )
{
	int i;
	plat_driver_entity_t *driver_entity = platform_driver_create(compatible, drv_direct, drv_type, extra_val, match_point_num, match_point_compatibles);
	plat_event_entity_t *event_entity = NULL;
	case_cell_t case_cell;
	
	if(NULL == driver_entity)
		return;
	if(extra_val != 0)
		platform_bus_match_devdrv(&device_list, driver_entity, INIT_EVENT_LIST_ID);
	else {
		if(drv_in == drv_direct) 
			platform_bus_match_devdrv(&device_list, driver_entity, IN_EVENT_LIST_ID);
		else if(drv_out == drv_direct) 
			event_entity = platform_bus_match_devdrv(&device_list, driver_entity, OUT_EVENT_LIST_ID);
		else;
	}
	memset(&case_cell, 0, sizeof(case_cell_t));
	case_cell.match_point_num = match_point_num;
	for(i = 0; i < match_point_num; i++)
		case_cell.match_points[i] = (platform_device_get_entity_v_compatible(&device_list, match_point_compatibles[i]))->dev_id;
	platform_event_add_case(event_entity, time_range[0], time_range[1], &case_cell);
	util_free(driver_entity);
}
