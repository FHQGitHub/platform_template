#include <stdio.h>
#include "state_process.h"
#include "platform_event.h"
#include "platform_bus.h"
#include "drv_SI446x.h"

static void paltform_event_input_hook(plat_event_entity_t *event_entity);
static void paltform_event_output_hook(plat_event_entity_t *event_entity);

plat_event_list_package_t event_list_package;

plat_event_list_t *plat_event_get_list_v_id(int event_list_id)
{
	int i;
	for(i = 0; i < event_list_package.list_num; i++) {
		if(event_list_package.list_entity[i]->ev_list_id == event_list_id)
			return event_list_package.list_entity[i];
	}
	return NULL;
}

static plat_event_entity_t *platform_event_get_entity_v_id(int event_list_id , int ev_id)
{
	struct list_head *t = NULL;
        plat_event_entity_t *p = NULL;
	plat_event_list_t *event_list = NULL;
	
	event_list = plat_event_get_list_v_id(event_list_id);
	
	if(!ASSERT_UTILS(event_list))
		return NULL;
	
	list_for_each(t, event_list->event_head) {
		p = list_entry(t, plat_event_entity_t);
		if(p->ev_id == ev_id)
			return p;
	}
	return NULL;
}

static void paltform_event_input_set_cells(plat_event_entity_t *event_entity)
{
	int i, j = 0;
	struct list_head *t, *s = NULL;
        plat_event_entity_t *p, *q = NULL;
	plat_event_case_t *m, *n = NULL;
	
	list_for_each(t, event_entity->ev_case_head) {
		m = list_entry(t, plat_event_case_t);
		for(i = 0; i < m->case_cell.match_point_num; i++) {
			p = platform_event_get_entity_v_id(OUT_EVENT_LIST_ID, m->case_cell.match_points[i]);
			list_for_each(s, p->ev_case_head) {
				n = list_entry(s, plat_event_case_t);
				for(j = 0; (j < 10) && (n->case_cell.match_points[j] != 0); j++) {
					if((n->case_cell.match_points[j] & (~0x8000)) == event_entity->ev_id) {
						n->case_cell.match_points[j] |= 0x8000;
						break;
					}	
				}
			}
		}
	}
}

void platform_event_handle_hw_status()
{
	struct list_head *t = NULL;
	plat_event_entity_t *p = NULL;
	plat_kobject_t *q = NULL;
	plat_event_list_t *event_list;
	int need_update = 0;
	int new_dev_value;
	
	event_list = plat_event_get_list_v_id(IN_EVENT_LIST_ID);
	if(NULL == event_list)
		return;
		
	list_for_each(t, event_list->event_head) {
		p = list_entry(t, plat_event_entity_t);
		if(p->kobj_kernel->dev_kernel->dev_in_value[1] == 0 && p->kobj_kernel->dev_kernel->dev_in_value[0] == 1) {
			if(drv_trigger_high == p->kobj_kernel->drv_kernel->drv_type) {
//				paltform_event_input_set_cells(p);
//				platform_debug("event <%s> triggered.\r\n", p->ev_compatible);
				position_change((p->kobj_kernel->dev_kernel->dev_extra_value) & 0x00ff, 1);
				si4463_send_data[0] = ((p->kobj_kernel->dev_kernel->dev_extra_value) & 0xff00) >> 8;
				need_update = 1;
			}
		}
		else if(p->kobj_kernel->dev_kernel->dev_in_value[1] == 1 && p->kobj_kernel->dev_kernel->dev_in_value[1] == 0) {
			if(drv_trigger_low == p->kobj_kernel->drv_kernel->drv_type) {
//				paltform_event_input_set_cells(p);
//				platform_debug("event <%s> triggered.\r\n", p->ev_compatible);
				position_change((p->kobj_kernel->dev_kernel->dev_extra_value) & 0x00ff, 1);
				si4463_send_data[0] = ((p->kobj_kernel->dev_kernel->dev_extra_value) & 0xff00) >> 8;
				need_update = 1;
			}
			else
				position_change(p->kobj_kernel->dev_kernel->dev_extra_value, 0);
		}
		p->kobj_kernel->dev_kernel->dev_in_value[1] = p->kobj_kernel->dev_kernel->dev_in_value[0];
	}
	
	event_list = plat_event_get_list_v_id(OUT_EVENT_LIST_ID);
	if(NULL == event_list)
		return;
	
	list_for_each(t, kobject_list->kobj_head) {
		q = list_entry(t, plat_kobject_t);
		platform_kobject_board_write(&(q->kernel), q->kernel.dev_kernel->dev_out_value);
	}
	if(need_update) {
		SI446x_Send_Packet( (uint8_t *)si4463_send_data, 4, 0, 0 );
		printf("si446x send data: %d %d %d %d\r\n", si4463_send_data[0], si4463_send_data[1], si4463_send_data[2], si4463_send_data[3]);
	}
}

static void paltform_event_input_hook(plat_event_entity_t *event_entity)
{
	int i;
	switch(event_entity->kobj_kernel->drv_kernel->drv_type) {
		case drv_trigger_high:
			if(1 == event_entity->kobj_kernel->dev_kernel->dev_in_value[0]) {
				paltform_event_input_set_cells(event_entity);
				platform_debug("event <%s> triggered.\r\n", event_entity->ev_compatible);
			}
			else;
		break;
		case drv_trigger_low:
			if(0 == event_entity->kobj_kernel->dev_kernel->dev_in_value[0]) {
				paltform_event_input_set_cells(event_entity);
				platform_debug("event <%s> triggered.\r\n", event_entity->ev_compatible);
			}
			else;
		break;
			
		case drv_equals_low:
			if(0 == event_entity->kobj_kernel->dev_kernel->dev_out_value) {
				paltform_event_input_set_cells(event_entity);
				platform_debug("event <%s> triggered.\r\n", event_entity->ev_compatible);
			}
			else;
		break;
		
		case drv_equals_high:
			if(1 == event_entity->kobj_kernel->dev_kernel->dev_out_value) {
				paltform_event_input_set_cells(event_entity);
				platform_debug("event <%s> triggered.\r\n", event_entity->ev_compatible);
			}
			else;
		break;
		case drv_analog:
			
		break;
		default:
			
		break;
		
	}
}

static void paltform_event_output_hook(plat_event_entity_t *event_entity)
{
	int i = 0;
	struct list_head *t = NULL;
	plat_event_case_t *q = NULL;
	int stage_status = -1;
	
	list_for_each(t, event_entity->ev_case_head) {
		q = list_entry(t, plat_event_case_t);
		stage_status = 0;
		for(i = 0; i < q->case_cell.match_point_num; i++) {
			if(0 == (q->case_cell.match_points[i] & 0x8000))
				stage_status = -1;
		}
		if(stage_status == 0)
			break;
	}
	
	if(0 == stage_status) {
		
		switch(event_entity->kobj_kernel->drv_kernel->drv_type) {
			case drv_trigger_high:
				event_entity->kobj_kernel->dev_kernel->dev_out_value = 1;
//				dops.write(event_entity->dev_kernel->dev_board_attr.GPIO, event_entity->dev_kernel->dev_board_attr.GPIO_Pin, event_entity->drv_kernel->drv_type, 1);
			break;
			case drv_trigger_low:
				event_entity->kobj_kernel->dev_kernel->dev_out_value = 0;
				
//				dops.write(event_entity->dev_kernel->dev_board_attr.GPIO, event_entity->dev_kernel->dev_board_attr.GPIO_Pin, event_entity->drv_kernel->drv_type, 0);
			break;
			case drv_trigger_flip:
				event_entity->kobj_kernel->dev_kernel->dev_out_value = !(event_entity->kobj_kernel->dev_kernel->dev_out_value);
			break;
			case drv_analog:

			break;
			default:
				
			break;
				
		}
		platform_debug("trigger event <%s>.\r\n", event_entity->ev_compatible);
		

	}
	
	list_for_each(t, event_entity->ev_case_head) {
		q = list_entry(t, plat_event_case_t);
		for(i = 0; i < q->case_cell.match_point_num; i++)
			q->case_cell.match_points[i] &= ~0x8000;
			
	}
}

int platform_event_construct_list(int event_list_id)
{
	EVENT_CREATE_UTILS(event_list, plat_event_list_t);
	if(!ASSERT_UTILS(event_list))
		return -1;
	
	LIST_CREATE_UTILS(event_list->event_head, event_list);
	if(!ASSERT_UTILS(event_list->event_head)) {
		util_free(event_list);
		return -1;
	}
	list_head_init(event_list->event_head);
	event_list->ev_list_id = event_list_id;
	event_list->event_num = 0;
	
	event_list_package.list_entity[event_list_package.list_num] = event_list;
	event_list_package.list_num++;
	
	return 0;
}

void platform_event_destruct_list(int event_list_id)
{
	plat_event_list_t *event_list = NULL;
}

int platform_event_add_case(plat_event_entity_t *event_entity, int time_rangeA, int time_rangeB, const case_cell_t *case_cell)
{
	if(!ASSERT_UTILS(event_entity))
		return -1;
	EVENT_CREATE_UTILS(new_case, plat_event_case_t);
	if(!ASSERT_UTILS(new_case))
		return NULL;
	memset(new_case, 0, sizeof(plat_event_case_t));
	LIST_CREATE_UTILS(new_case->case_entry, new_case);
	if(!ASSERT_UTILS(new_case->case_entry)) {
		util_free(new_case);
		return -1;
	}
	
	new_case->time_range[0] = time_rangeA;
	new_case->time_range[0] = time_rangeB;
	new_case->case_id = event_entity->ev_case_num;
	++(event_entity->ev_case_num);
	memcpy(&new_case->case_cell, case_cell, sizeof(case_cell_t));
	
	list_add_tail(new_case->case_entry, event_entity->ev_case_head);
	
	return 0;
}

plat_event_entity_t *platform_event_create()
{
	EVENT_CREATE_UTILS(new_entity, plat_event_entity_t);
	if(!ASSERT_UTILS(new_entity))
		return NULL;
	
	util_memset(new_entity, 0, sizeof(plat_event_entity_t));
	LIST_CREATE_UTILS(new_entity->ev_entry, new_entity);
	if(!ASSERT_UTILS(new_entity->ev_entry)) {
		util_free(new_entity);
		return NULL;
	}
	LIST_CREATE_UTILS(new_entity->ev_case_head, new_entity);
	if(!ASSERT_UTILS(new_entity->ev_case_head)) {
		util_free(new_entity->ev_entry);
		util_free(new_entity);
		return NULL;
	}
	return new_entity;
}

void platform_event_destroy(plat_event_entity_t *event_entity)
{

}


void platform_event_register(int event_list_id, plat_event_entity_t *new_entity)
{
	plat_event_list_t *event_list = NULL;
	
	if(!ASSERT_UTILS(new_entity))
		return;
	
	event_list = plat_event_get_list_v_id(event_list_id);
	if(NULL == event_list)
		return;
	
	new_entity->ev_id = event_list->event_num + 1;
	list_add_tail(new_entity->ev_entry, event_list->event_head);
	
	list_head_init(new_entity->ev_case_head);
	
	++event_list->event_num;
}

plat_event_entity_t *platform_event_get_entity_v_compatible(const plat_event_list_t *event_list, const char *ev_compatible)
{
	struct list_head *t = NULL;
        plat_event_entity_t *p = NULL;

	if(!ASSERT_UTILS(event_list))
		return NULL;
	
	list_for_each(t, event_list->event_head) {
		p = list_entry(t, plat_event_entity_t);
		if(!strcmp(p->ev_compatible, ev_compatible))
			return p;
	}
	return NULL;
}

void platform_event_print_list(int event_list_id)
{
	int i;
	struct list_head *t, *s = NULL;
        plat_event_entity_t *p;
	plat_event_case_t *q = NULL;
	plat_event_list_t *event_list = NULL;
		
	event_list = plat_event_get_list_v_id(event_list_id);
	if(NULL == event_list)
		return;
	
	printf("\r\nevent list id:%d\r\n", event_list->ev_list_id);
	printf("list entity number:%d\r\n", event_list->event_num);
	list_for_each(t, event_list->event_head) {
		printf("------------------------------------------------\r\n");
		p = list_entry(t, plat_event_entity_t);
		printf("\t<event id>:%d\r\n", p->ev_id);
		printf("\t<event compatible>:%s\r\n", p->ev_compatible);
		printf("\t<event match list id>:%d\r\n", p->ev_match_list_id);
		if(p->kobj_kernel->drv_kernel->drv_direct == drv_in)
			printf("\t<event direct>:input\r\n");
		else if(p->kobj_kernel->drv_kernel->drv_direct == drv_out)
			printf("\t<event direct>:output\r\n");
		switch(p->kobj_kernel->drv_kernel->drv_type) {
			case drv_trigger_high : printf("\t<event type>: trigger high\r\n"); break;
			case drv_trigger_low : printf("\t<event type>: trigger low\r\n"); break;
			case drv_trigger_flip : printf("\t<event type>: trigger flip\r\n"); break;
			case drv_equals_high : printf("\t<event type>: equals high\r\n"); break;
			case drv_equals_low : printf("\t<event type>: equals low\r\n"); break;
			case drv_analog : printf("\t<event type>: analog\r\n"); break;
			default: printf("\t<event type>: unknown\r\n"); break;
		}
		printf("\t<event board attribution>:\r\n");
		switch((uint32_t)p->kobj_kernel->dev_kernel->dev_board_attr.GPIO) {
			case GPIOA_BASE: printf("\t\t<GPIO>: GPIOA\r\n"); break;
			case GPIOB_BASE: printf("\t\t<GPIO>: GPIOB\r\n"); break;
			case GPIOC_BASE: printf("\t\t<GPIO>: GPIOC\r\n"); break;
			case GPIOD_BASE: printf("\t\t<GPIO>: GPIOD\r\n"); break;
			case GPIOE_BASE: printf("\t\t<GPIO>: GPIOE\r\n"); break;
			case GPIOF_BASE: printf("\t\t<GPIO>: GPIOF\r\n"); break;
			case GPIOG_BASE: printf("\t\t<GPIO>: GPIOG\r\n"); break;
			default: printf("\t\t<GPIO>: unknown\r\n"); break;
		}
		printf("\t\t<Pin>: %d\r\n", p->kobj_kernel->dev_kernel->dev_board_attr.GPIO_Pin); 
		printf("\t<event case number>:%d\r\n", p->ev_case_num);
		list_for_each(s, p->ev_case_head) {
			q = list_entry(s, plat_event_case_t);
			printf("\t<event case %d>:\r\n", q->case_id);
			printf("\t----------------------------------------\r\n");
			printf("\t\t<case match point number>:%d\r\n", q->case_cell.match_point_num);
			printf("\t\t<case match points>: ");
			for(i = 0; i < q->case_cell.match_point_num; i++)
				printf("%d ", q->case_cell.match_points[i]);
			printf("\r\n");
			printf("\t\t<case match point compatible>: ");
			for(i = 0; i < q->case_cell.match_point_num; i++)
				printf("%s ", q->case_cell.match_point_compatible[i]);
			printf("\r\n");
			printf("\t\t<case time range>: <%d:%d>\r\n", q->time_range[0], q->time_range[1]);
		}
		
	}
}

static void platform_event_handle_entity(plat_event_entity_t *event_entity)
{
	if(drv_in == event_entity->kobj_kernel->drv_kernel->drv_direct)
		paltform_event_input_hook(event_entity);
	else if(drv_out == event_entity->kobj_kernel->drv_kernel->drv_direct)
		paltform_event_output_hook(event_entity);
}

void platform_event_handle_list(int event_list_id) 
{
	struct list_head *t = NULL;
	plat_event_list_t *event_list = NULL;
	
	event_list = plat_event_get_list_v_id(event_list_id);
	if(NULL == event_list)
		return;
	
	if(!ASSERT_UTILS(event_list))
		return;
	
	list_for_each(t, event_list->event_head)
		platform_event_handle_entity(list_entry(t, plat_event_entity_t));
}







