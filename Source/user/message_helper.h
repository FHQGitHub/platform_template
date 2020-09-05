#ifndef __MESSAGE_HELPER_H
#define __MESSAGE_HELPER_H

#include "platform_bus.h"

#define MESSAGE_TYPE_NORMAL	0
#define MESSAGE_TYPE_INIT	1
#define MESSAGE_TYPE_ERR	-1

#define MESSAGE_CMD_TYPE_SET	0
#define MESSAGE_CMD_TYPE_FLIP	1
#define MESSAGE_CMD_TYPE_ERR	-1

#define MAX_OBJECT_LIST_NUM	2
#define IN_OBJECT_LIST		0
#define OUT_OBJECT_LIST		1

#define OBJECT_CREATE_UTILS(name, type)    type *name = util_alloc(1, type)

typedef struct {
	char compatible[20];
	enum plat_driver_direct obj_direct;   	// in out
	enum plat_driver_type obj_type;
	int extra_val;
}object_entity_t;

typedef struct {
	int object_num;
	object_entity_t object_entity[20];
}object_package_t;

void message_get_pair(const char *message, const char *token, char **in_pair, char **out_pair);
object_package_t *message_handle_pair(const char *message_pair, enum plat_driver_direct object_direct);
int message_helper(const char *raw_message_string);

#endif
