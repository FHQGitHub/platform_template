#include "platform_event.h"
#include "stdio.h"

static void paltform_event_input_hook(plat_event_entity_t *event_entity);
static void paltform_event_output_hook(plat_event_entity_t *event_entity);

plat_event_list_package_t event_list_package;

case_cell_t test_case_cell = {
	.match_points_num = 2,
	.match_points = {1, 2},
};

plat_event_list_t *plat_event_get_list_v_id(int event_list_id)
{
	int i;
	
	for(i = 0; i < event_list_package.list_num; i++) {
		if(event_list_package.list_entity[i].ev_list_id == event_list_id)
			return &(event_list_package.list_entity[i]);
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
	int i = 0;
	struct list_head *t = NULL;
        plat_event_entity_t *p = NULL;
	plat_event_case_t *q = NULL;
	
	p = platform_event_get_entity_v_id(OUT_EVENT_LIST_ID, event_entity->ev_id);
	t = p->ev_case_head;
	do {
		list_next(t);
		i++;
	}while(i < p->ev_case_stage);
	
	q = list_entry(t, plat_event_case_t);
	for(i = 0; i < 10; i++) {
		if((q->case_cell.match_points[i] & (~0x8000)) == event_entity->ev_id) {
			q->case_cell.match_points[i] |= 0x8000;
			break;
		}	
	}
}

static void paltform_event_input_hook(plat_event_entity_t *event_entity)
{
	int board_attr_status = 0;
	board_attr_status = dops.read(event_entity->ev_board_attr.GPIO, event_entity->ev_board_attr.GPIO_Pin, event_entity->ev_type);
	switch(event_entity->ev_type) {
		case trigger_high:
			if(1 == board_attr_status)
				paltform_event_input_set_cells(event_entity);
			else;
		break;
		case trigger_low:
			if(0 == board_attr_status)
				paltform_event_input_set_cells(event_entity);
			else;
		break;
		case analog:
			
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
	int stage_status = 0;
	
	t = event_entity->ev_case_head;
	do {
		list_next(t);
		i++;
	}while(i < event_entity->ev_case_stage);
	
	q = list_entry(t, plat_event_case_t);
	for(i = 0; i < event_entity->ev_case_num; i++) {
		if(0 == (q->case_cell.match_points[i] & 0x8000)) {
			stage_status = -1;
			break;
		}
	}
	
	//current stage is finished, enter next stage
	if(0 == stage_status) {
		for(i = 0; i < event_entity->ev_case_num; i++) {
			q->case_cell.match_points[i] &= ~0x8000;
			event_entity->ev_case_stage++;
		}
		if(event_entity->ev_case_stage == event_entity->ev_case_num) {
			switch(event_entity->ev_type) {
				case trigger_high:
					dops.write(event_entity->ev_board_attr.GPIO, event_entity->ev_board_attr.GPIO_Pin, event_entity->ev_type, 1);
				break;
				case trigger_low:
					dops.write(event_entity->ev_board_attr.GPIO, event_entity->ev_board_attr.GPIO_Pin, event_entity->ev_type, 0);
				break;
				case analog:

				break;
				default:
					
				break;
				
			}
			
			//data transmit function here
			
			event_entity->ev_case_stage = 0;
		}
	}
	else {
		switch(event_entity->ev_type) {
			case trigger_high:
				dops.write(event_entity->ev_board_attr.GPIO, event_entity->ev_board_attr.GPIO_Pin, event_entity->ev_type, 0);
			break;
			case trigger_low:
				dops.write(event_entity->ev_board_attr.GPIO, event_entity->ev_board_attr.GPIO_Pin, event_entity->ev_type, 1);
			break;
			case analog:

			break;
			default:
				
			break;
				
		}
	}
}

int platform_event_construct_list(int event_list_id)
{
	plat_event_list_t *event_list = NULL;
	
	if(!ASSERT_UTILS(event_list))
		return -1;
	
	event_list = plat_event_get_list_v_id(event_list_id);
	if(NULL == event_list)
		return NULL;
	
	LIST_CREATE_UTILS(event_list->event_head, event_list);
	if(!ASSERT_UTILS(event_list->event_head))
		return -1;
	list_head_init(event_list->event_head);
	event_list->ev_list_id = event_list_id;
	event_list->event_num = 0;
	
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
	++event_entity->ev_case_num;
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

void platform_event_register(int event_list_id, plat_event_entity_t *new_entity)
{
	plat_event_list_t *event_list = NULL;
	
	if(!ASSERT_UTILS(event_list) || !ASSERT_UTILS(new_entity))
		return;
	
	event_list = plat_event_get_list_v_id(event_list_id);
	if(NULL == event_list)
		return;
	
	new_entity->ev_id = event_list->event_num;
	list_add_tail(new_entity->ev_entry, event_list->event_head);
	
	list_head_init(new_entity->ev_case_head);
	
	++event_list->event_num;
	
//	platform_event_add_case(new_entity, 0, 500, &test_case_cell);
//	platform_event_add_case(new_entity, 0, 500, &test_case_cell);
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
	
	if(!ASSERT_UTILS(event_list))
		return;
	
	event_list = plat_event_get_list_v_id(event_list_id);
	if(NULL == event_list)
		return;
	
	printf("\r\nevent list id:%d\r\n", event_list->ev_list_id);
	printf("list entity number:%d\r\n", event_list->event_num);
	list_for_each(t, event_list->event_head) {
		printf("------------------------------------------------\r\n");
		p = list_entry(t, plat_event_entity_t);
		printf("\t<event id>:%d\r\n", p->ev_id);
		printf("\t<event match list id>:%d\r\n", p->ev_match_list_id);
		if(p->ev_direct == drv_in)
			printf("\t<event direct>:input\r\n");
		else if(p->ev_direct == drv_out)
			printf("\t<event direct>:output\r\n");
		printf("\t<event case number>:%d\r\n", p->ev_case_num);
		list_for_each(s, p->ev_case_head) {
			q = list_entry(s, plat_event_case_t);
			printf("\t<case %d>:\r\n", q->case_id);
			printf("\t----------------------------------------\r\n");
			printf("\t\t<case match point number>:%d\r\n", q->case_cell.match_points_num);
			printf("\t\t<match points>: ");
			for(i = 0; i < q->case_cell.match_points_num; i++)
				printf("%d ", q->case_cell.match_points[i]);
			printf("\r\n");
			printf("\t\t--------------------------------\r\n");
			printf("\t\t\t<time range>: <%d:%d>\r\n", q->time_range[0], q->time_range[1]);
		}
		printf("\t<event board attribution>:\r\n");
		switch((uint32_t)p->ev_board_attr.GPIO) {
			case GPIOA_BASE: printf("\t\t<GPIO>: GPIOA\r\n"); break;
			case GPIOB_BASE: printf("\t\t<GPIO>: GPIOB\r\n"); break;
			case GPIOC_BASE: printf("\t\t<GPIO>: GPIOC\r\n"); break;
			case GPIOD_BASE: printf("\t\t<GPIO>: GPIOD\r\n"); break;
			case GPIOE_BASE: printf("\t\t<GPIO>: GPIOE\r\n"); break;
			case GPIOF_BASE: printf("\t\t<GPIO>: GPIOF\r\n"); break;
			case GPIOG_BASE: printf("\t\t<GPIO>: GPIOG\r\n"); break;
			default: printf("\t\t<GPIO>: unknown\r\n"); break;
		}
		printf("\t\t<Pin>: %d\r\n", p->ev_board_attr.GPIO_Pin); 
	}
}

static void platform_event_handle_entity(plat_event_entity_t *event_entity)
{
	if(drv_in == event_entity->ev_direct)
		paltform_event_input_hook(event_entity);
	else if(drv_out == event_entity->ev_direct)
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







