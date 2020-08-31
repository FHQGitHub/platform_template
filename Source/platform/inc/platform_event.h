#ifndef __PLATFORM_EVENT_H
#define __PLATFORM_EVENT_H

#include "list_utils.h"
#include "stm32f4xx.h"
#include "platform_device.h"
#include "platform_driver.h"

#define MAX_EVENT_LIST_NUM	3

#define INIT_EVENT_LIST_ID	0
#define IN_EVENT_LIST_ID	1
#define OUT_EVENT_LIST_ID	2

#define EVENT_CREATE_UTILS(name, type)    type *name = util_alloc(1, type)

typedef struct {
	uint16_t match_points_num;
	uint16_t match_points[10];
}case_cell_t;

typedef struct {
	int time_range[2];
	int case_id;
	case_cell_t case_cell;
	struct list_head *case_entry;
}plat_event_case_t;

typedef struct event_entity{
	char ev_compatible[20];  		//store name £ºconx 
	int ev_id;            			//id for conx 
	int ev_match_list_id;   		//match other list in--out
	int ev_case_num;
	int ev_case_stage;
	enum plat_driver_direct ev_direct;   	// in out
	enum plat_driver_type ev_type;
	plat_device_attr_t ev_board_attr;    	//GPIOX Pinx
	struct list_head *ev_entry;
	struct list_head *ev_case_head;
}plat_event_entity_t;

typedef struct {
	int ev_list_id;
	int event_num;
	struct list_head *event_head;
}plat_event_list_t;

typedef struct {
	int list_num;
	plat_event_list_t list_entity[MAX_EVENT_LIST_NUM];
}plat_event_list_package_t;
	
extern plat_event_list_package_t event_list_package;

plat_event_list_t *plat_event_get_list_v_id(int event_list_id);
int platform_event_construct_list(int event_list_id);
plat_event_entity_t *platform_event_create(void);
void platform_event_register(int event_list_id, plat_event_entity_t *new_entity);
plat_event_entity_t *platform_event_get_entity_v_compatible(const plat_event_list_t *event_list, const char *ev_compatible);
int platform_event_add_case(plat_event_entity_t *event_entity, int time_rangeA, int time_rangeB, const case_cell_t *case_cell);
void platform_event_print_list(int event_list_id);
void platform_event_handle_list(int event_list_id);

#endif
