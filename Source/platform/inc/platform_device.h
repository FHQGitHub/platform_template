#ifndef __PLATFORM_DEVICE_H
#define __PLATFORM_DEVICE_H

#include "list_utils.h"
#include "stm32f4xx.h"

#define DEVICE_CREATE_UTILS(name, type)    type *name = util_alloc(1, type)

typedef struct {
	GPIO_TypeDef *GPIO;
	uint16_t GPIO_Pin;
}plat_device_attr_t;

typedef struct {
	char dev_compatible[20];
	int dev_id;
	plat_device_attr_t dev_board_attr;
	struct list_head *dev_entry;
}plat_device_entity_t;

typedef struct {
	int dev_list_id;
	int device_num;
	struct list_head *device_head;
}plat_device_list_t;

extern plat_device_list_t device_list;

int platform_device_construct_list(plat_device_list_t *device_list, int list_id);
int platform_device_register(plat_device_list_t *device_list, const plat_device_entity_t *pNewdeviceEntity);
plat_device_entity_t *platform_device_get_entity_v_compatible(const plat_device_list_t *device_list, const char *dev_compatible);
plat_device_attr_t *platform_device_get_board_attr_v_compatible(const plat_device_list_t *device_list, const char *dev_compatible);
void platform_device_parse_configuration(const char *conf_json_string);
char *platform_device_generate_emu_configuration(void);
void platform_device_print_list(const plat_device_list_t *device_list);

#endif
