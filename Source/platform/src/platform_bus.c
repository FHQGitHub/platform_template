#include "platform_bus.h"
#include "platform_kobject.h"
#include "platform_event.h"

int init_flag;

static plat_kobject_t *platform_bus_generate_kobject(const plat_device_entity_t *device_entity, const plat_driver_entity_t *driver_entity)
{
	plat_kobject_t *new_kobject = NULL;
	if(!ASSERT_UTILS(device_entity) || !ASSERT_UTILS(driver_entity))
		return NULL;
	
	new_kobject = platform_kobject_create();
	if(NULL == new_kobject)
		return NULL;
	
	new_kobject->kernel.dev_kernel = (device_kernel_t *)&(device_entity->kernel);
	new_kobject->kernel.drv_kernel = util_alloc(1, driver_kernel_t);
	if(!ASSERT_UTILS(new_kobject->kernel.drv_kernel)) {
		platform_kobject_destroy();
		return NULL;
	}
	util_memcpy(new_kobject->kernel.drv_kernel, (void *)&(driver_entity->kernel), sizeof(driver_kernel_t));
	strcpy(new_kobject->compatible, device_entity->dev_compatible);
	return new_kobject;
}

static plat_event_entity_t *platform_bus_generate_event(const plat_kobject_t *kobject_entity)
{
	plat_event_entity_t *new_event = NULL;
	
	if(!ASSERT_UTILS(kobject_entity))
		return NULL;
	
	new_event = platform_event_create();
	if(NULL == new_event)
		return NULL;
	
	strcpy(new_event->ev_compatible, kobject_entity->compatible);
	switch(kobject_entity->kernel.drv_kernel->drv_type) {
		case drv_trigger_high : strcat(new_event->ev_compatible, "-TH"); break;
		case drv_trigger_low : strcat(new_event->ev_compatible, "-TL"); break;
		case drv_trigger_flip : strcat(new_event->ev_compatible, "-TF"); break;
		case drv_equals_high : strcat(new_event->ev_compatible, "-EH"); break;
		case drv_equals_low : strcat(new_event->ev_compatible, "-EL"); break;
		case drv_analog : strcat(new_event->ev_compatible, "-A"); break;
		default: break;
	}

	new_event->kobj_kernel = (kobject_kernel_t *)&(kobject_entity->kernel);
	return new_event;
}

static plat_event_entity_t *platform_bus_match_devdrv(const plat_device_list_t *device_list, const plat_driver_entity_t *driver_entity, int event_list_id)
{
	struct list_head *t = NULL;
        plat_device_entity_t *p = NULL;
	plat_event_entity_t *new_event = NULL;
	plat_event_entity_t *match_event = NULL;
	plat_kobject_t *match_kobject = NULL;
	plat_event_list_t *event_list = NULL;
	char *index = NULL;
	char compatible_temp[15] = {0};
	
	event_list = plat_event_get_list_v_id(event_list_id);
	if(NULL == event_list)
		return NULL;
	
	list_for_each(t, device_list->device_head) {
		p = list_entry(t, plat_device_entity_t);
		index = strstr(driver_entity->drv_compatible, "-");
		memset(compatible_temp, 0, sizeof(compatible_temp));
		memcpy(compatible_temp, driver_entity->drv_compatible, (int)(index - driver_entity->drv_compatible));
		if(!strcmp(p->dev_compatible, compatible_temp)) {
			match_kobject = platform_kobject_get_entity_v_compatible(kobject_list, compatible_temp);
			if(NULL == match_kobject) {
					match_kobject = platform_bus_generate_kobject(p, driver_entity);
				if(NULL == match_kobject)
					return NULL;
			}
			else;
			

			platform_kobject_register(kobject_list, match_kobject);
			match_event = platform_event_get_entity_v_compatible(event_list, driver_entity->drv_compatible);
			if(NULL == match_event) {
				new_event = platform_bus_generate_event(match_kobject);
				if(NULL != new_event)
					platform_event_register(event_list_id, new_event);
				match_event = new_event;
			}
			else;
			
			platform_kobject_board_init(&(match_kobject->kernel));
			return match_event;
		}
	}
	
	platform_debug("can't match driver with compatible <%s>.\r\n", driver_entity->drv_compatible);
	return NULL;
}

void platform_bus_probe(	const char 	*compatible, 			// STORE SET CON
				enum 		plat_driver_direct drv_direct, 	// SET-out W-in
				enum plat_driver_type drv_type,			//H L A
				int 		time_range[2],
				int 		match_point_num,
				char 		**match_point_compatibles,	//which con related to this con
				int 		extra_val
			)
{
	int i;
	int event_list_id = 0;
	plat_driver_entity_t *driver_entity = NULL;
	plat_device_entity_t *device_entity = NULL;
	plat_event_entity_t *event_entity = NULL;
	plat_event_entity_t *match_event_entity = NULL;
	case_cell_t case_cell;
	
	if(NULL == compatible || NULL == match_point_compatibles)
		return;
	
	driver_entity = platform_driver_create(compatible, drv_direct, drv_type, extra_val, match_point_num, match_point_compatibles);
	if(NULL == driver_entity)
		return;
	
	if(extra_val != 0)
		event_list_id = INIT_EVENT_LIST_ID;
	else {
		if(drv_in == drv_direct) 
			event_list_id = IN_EVENT_LIST_ID;
		else if(drv_out == drv_direct) 
			event_list_id = OUT_EVENT_LIST_ID;
		else;
	}
	
	event_entity = platform_bus_match_devdrv(device_list, driver_entity, event_list_id);
	
	memset(&case_cell, 0, sizeof(case_cell_t));
	case_cell.match_point_num = match_point_num;
	case_cell.match_point_compatible = util_alloc(match_point_num, char *);
	if(NULL == case_cell.match_point_compatible)
		platform_error("memory alloc fail.\r\n");
	for(i = 0; i < match_point_num; i++) {
		*((case_cell.match_point_compatible) + i) = util_alloc(20, char);
		if(NULL == *((case_cell.match_point_compatible) + i))
			platform_error("memory alloc fail.\r\n");
		else {
			util_memcpy(*((case_cell.match_point_compatible) + i), 0, 20);
			strcpy(*((case_cell.match_point_compatible) + i), *(match_point_compatibles + i));
		}
	}
	
	if(NULL == time_range)
		platform_event_add_case(event_entity, 0, 0, &case_cell);
	else
		platform_event_add_case(event_entity, time_range[0], time_range[1], &case_cell);
	
	util_free(driver_entity);
}

void platform_bus_match_list(int event_list_id_1, int event_list_id_2)
{
	int i, j, k;
	struct list_head *t, *s, *v, *u= NULL;
	plat_event_entity_t *p, *q = NULL;
	plat_event_case_t *m, *n = NULL;
	plat_event_list_t *event_list[2] = {NULL};
	plat_event_entity_t *event_entity = NULL;
	plat_event_entity_t *match_event_entity = NULL;
	
	event_list[0] = plat_event_get_list_v_id(event_list_id_1);
	if(NULL == event_list[0])
		return;
	
	event_list[1] = plat_event_get_list_v_id(event_list_id_2);
	if(NULL == event_list[1])
		return;
	
	/*traverse event list<1> entity*/
	list_for_each(t, event_list[0]->event_head) {
		p = list_entry(t, plat_event_entity_t);
		/*traverse event list<1> entity<n> case*/
		list_for_each(s, p->ev_case_head) {
			m = list_entry(s, plat_event_case_t);
			/*traverse event list<1> entity<n> case<n> match points*/
			for(i = 0; i < m->case_cell.match_point_num; i++) {
				/*compare all entities in the symmetry list*/
				list_for_each(v, event_list[1]->event_head) {
					/*traverse event list<2> entity*/
					q = list_entry(v, plat_event_entity_t);
					/*find list<1> entity<n> case<n> match points*/
					if(!strcmp(q->ev_compatible, m->case_cell.match_point_compatible[i]))
						m->case_cell.match_points[i] = q->ev_id;
				}
			}
			
		}
			
	}
	
	/*traverse event list<2> entity*/
	list_for_each(t, event_list[1]->event_head) {
		p = list_entry(t, plat_event_entity_t);
		/*traverse event list<2> entity<n> case*/
		list_for_each(s, p->ev_case_head) {
			m = list_entry(s, plat_event_case_t);
			/*traverse event list<2> entity<n> case<n> match points*/
			for(i = 0; i < m->case_cell.match_point_num; i++) {
				/*compare all entities in the symmetry list*/
				list_for_each(v, event_list[0]->event_head) {
					/*traverse event list<2> entity*/
					q = list_entry(v, plat_event_entity_t);
					/*find list<1> entity<n> case<n> match points*/
					if(!strcmp(q->ev_compatible, m->case_cell.match_point_compatible[i]))
						m->case_cell.match_points[i] = q->ev_id;
				}
			}
			
		}
			
	}
}
