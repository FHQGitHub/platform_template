#ifndef __PLATFORM_KOBJECT_H
#define __PLATFORM_KOBJECT_H

#include "platform_device.h"
#include "platform_driver.h"

typedef struct {
	device_kernel_t *dev_kernel;
	driver_kernel_t *drv_kernel;
}kobject_kernel_t;

typedef struct {
	char compatible[20];
	kobject_kernel_t kernel;
	struct list_head *kobj_entry;
}plat_kobject_t;

typedef struct {
	int kobj_list_id;
	int kobj_num;
	struct list_head *kobj_head;
}plat_kobject_list_t;

extern plat_kobject_list_t *kobject_list;

plat_kobject_t *platform_kobject_create(void);
void platform_kobject_destroy(void);
plat_kobject_list_t *platform_kobject_construct_list(int kobject_list_id);
plat_kobject_list_t *platform_kobject_destruct_list(int kobject_list_id);
void platform_kobject_register(plat_kobject_list_t *kobject_list, plat_kobject_t *new_kobject);
plat_kobject_t *platform_kobject_get_entity_v_compatible(const plat_kobject_list_t *kobject_list, const char *kobj_compatible);

void platform_kobject_board_init(const kobject_kernel_t *kobject_kernel);
uint16_t platform_kobject_board_read(const kobject_kernel_t *kobject_kernel);
void platform_kobject_board_write(const kobject_kernel_t *kobject_kernel, int val);
void platform_kobject_get_hw_status(const plat_kobject_list_t *kobject_list);

#endif
