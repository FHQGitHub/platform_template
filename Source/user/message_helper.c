#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "message_helper.h"

static void omit_extra_space(const char *raw_message, char *new_message, int size)
{
	int i, j;
	char temp[size];
	
	if(size <= 0)
		return;
	
	memset(temp, 0, size);
	
	/*掐头*/
	for(i = 0; i < size; i++) {
		if(raw_message[i] != ' ')
			break;
	}
	
	/*去尾*/
	for(j = size - 1; j >= 0; j--) {
		if(raw_message[j] != ' ')
			break;
	}
	strncpy(temp, raw_message + i, j - i + 1);
	
	j = 1;
	/*再去掉中间连续的空格*/
	for(i = 1; i < size; i++) {
		if(!(temp[i] == ' ' && temp[i - 1] == ' '))
			new_message[j++] = temp[i];
	}
	new_message[0] = temp[0];
}

static int is_number(char *string, int *number)
{
	int i, str_len;
	
	if(NULL == string)
		return 0;
	
	str_len = strlen(string);
	for(i = 0; i < str_len; i++) {
		if(!isdigit(string[i]))
			return 0;
	}
	*number = atoi(string);
	
	return 1;
}

static object_package_t *object_construct_package(int object_num)
{
	OBJECT_CREATE_UTILS(object_package, object_package_t);
	if(!ASSERT_UTILS(object_package))
		return NULL;

	util_memset(object_package, 0, sizeof(object_package_t));
	return object_package;
}


static int message_get_type(const char *message_string)
{
	if(NULL == message_string)
		return MESSAGE_TYPE_ERR;
	if(strstr(message_string, "WHEN"))
		return MESSAGE_TYPE_NORMAL;
	else
		return MESSAGE_TYPE_INIT;
}

static int message_get_cmd_type(const char *message_string)
{
	if(NULL == message_string)
		return MESSAGE_CMD_TYPE_ERR;
	if(strstr(message_string, "SET"))
		return MESSAGE_CMD_TYPE_SET;
	else if(strstr(message_string, "FLIP"))
		return  MESSAGE_CMD_TYPE_FLIP;
	else
		return MESSAGE_CMD_TYPE_ERR;
}

void message_get_pair(const char *message, const char *token, char **in_pair, char **out_pair)
{
	int in_pair_len, out_pair_len;
	char *split_point = NULL;
	char *in_pair_index = NULL;
	char *out_pair_index = NULL;
	
	if(NULL == message || NULL == token)
		return;
	
	if(*in_pair != NULL || *out_pair != NULL)
		return;
	
	split_point = strstr(message, token);
	if(NULL == split_point)
		return;
	in_pair_index = split_point + strlen(token) + 1;
	in_pair_len = strlen(in_pair_index);
	out_pair_index = (char *)message;
	out_pair_len = (int)(split_point - message) - 1;
	
	*in_pair = calloc(in_pair_len + 1, sizeof(char));
	*out_pair = calloc(out_pair_len + 1, sizeof(char));
	memcpy(*in_pair, in_pair_index, in_pair_len);
	memcpy(*out_pair, out_pair_index, out_pair_len);
}

static void message_print_object(const object_package_t *object_package)
{
	int i;
	
	printf("\r\nobject number: %d\r\n", object_package->object_num);
	for(i = 0; i < object_package->object_num; i++) {	
		printf("object %d:\r\n", i);
		printf("\tcompatible: %s\r\n", object_package->object_entity[i].compatible);
		printf("\tdirection: %d\r\n", object_package->object_entity[i].obj_direct);
		printf("\ttype: %d\r\n", object_package->object_entity[i].obj_type);
		printf("\textra value: %d\r\n", object_package->object_entity[i].extra_val);
	}
}

object_package_t *message_handle_pair(const char *message_pair, enum plat_driver_direct object_direct)
{
	int i;
	int AND_token_num = 0;
	int object_token_num = 0;
	int value_token_num = 0;
	int message_cmd_type;
	
	char temp[5] = {0};
	char *index_head = (char *)message_pair;
	char *index_tail = (char *)message_pair;
	object_package_t *object_package_pair = NULL;
	
	
	
	if(NULL == message_pair)
		return NULL;
	
	message_cmd_type = message_get_cmd_type(message_pair);
	index_head = (char *)message_pair;
	do {
		index_head = strstr(index_head, "#");
		if(index_head != NULL) {
			++object_token_num;
			index_head = index_head + 1;
		}
	} while(index_head != NULL);
	if(0 == object_token_num)
		goto message_syntax_error;

	index_head = (char *)message_pair;
	do {
		index_head = strstr(index_head, "@");
		if(index_head != NULL) {
			++value_token_num;
			index_head = index_head + 1;
		}
	} while(index_head != NULL);
	
	index_head = (char *)message_pair;
	do {
		index_head = strstr(index_head, "=");
		if(index_head != NULL) {
			++value_token_num;
			index_head = index_head + 1;
		}
	} while(index_head != NULL);
	
	index_head = (char *)message_pair;
	do {
		index_head = strstr(index_head, "AND");
		if(index_head != NULL) {
			++AND_token_num;
			index_head = index_head + 3;
		}
	} while(index_head != NULL);
	
	if(message_cmd_type != MESSAGE_CMD_TYPE_FLIP) {
		if(object_token_num != value_token_num || AND_token_num != object_token_num - 1)
			goto message_syntax_error;
		
		object_package_pair = object_construct_package(object_token_num);
		if(NULL == object_package_pair) {
			platform_error("memory alloc fail.\r\n");
			return 0;
		}
		
		index_head = (char *)message_pair;
		for(i = 0; i < object_token_num; i++) {
			index_head = strstr(index_head, "#") + 1;
			index_tail = strstr(index_head, " ");
			if(NULL == index_tail)
				goto message_syntax_error;
			util_memcpy(object_package_pair->object_entity[i].compatible, index_head, (int)(index_tail - index_head));
			
			
			index_head = index_tail + 1;
			if('@' != *index_head && '=' != *index_head)
				goto message_syntax_error;
			
			index_tail = strstr(index_head, " ");
			if(NULL == index_tail)
				strcpy(temp, index_head + 1);
			else
				util_memcpy(temp, index_head + 1, (int)(index_tail - index_head) - 1);
			
			if('@' == *index_head) {
				if(!strcmp(temp, "HIGH"))
					object_package_pair->object_entity[i].obj_type = drv_trigger_high;
				else if(!strcmp(temp, "LOW"))
					object_package_pair->object_entity[i].obj_type = drv_trigger_low;
				else {
					if(is_number(temp, &object_package_pair->object_entity[i].extra_val))
						object_package_pair->object_entity[i].obj_type = drv_analog;
					else
						goto message_syntax_error;
				}
			}
			else {
				if(!strcmp(temp, "HIGH"))
					object_package_pair->object_entity[i].obj_type = drv_equals_high;
				else if(!strcmp(temp, "LOW"))
					object_package_pair->object_entity[i].obj_type = drv_equals_low;
				else {
					if(is_number(temp, &object_package_pair->object_entity[i].extra_val))
						object_package_pair->object_entity[i].obj_type = drv_analog;
					else
						goto message_syntax_error;
				}
			}
			util_memset(temp, 0, sizeof(temp));
			object_package_pair->object_entity[i].obj_direct = object_direct;
		}
		object_package_pair->object_num = object_token_num;
	}
	else {
		if(value_token_num != 0 || AND_token_num != object_token_num - 1)
			goto message_syntax_error;
		
		object_package_pair = object_construct_package(object_token_num);
		if(NULL == object_package_pair) {
			platform_error("memory alloc fail.\r\n");
			return 0;
		}
		
		index_head = (char *)message_pair;
		for(i = 0; i < object_token_num; i++) {
			index_head = strstr(index_head, "#") + 1;
			index_tail = strstr(index_head, " ");
			if(NULL == index_tail)
				strcpy(object_package_pair->object_entity[i].compatible, index_head);
			else
				strncpy(object_package_pair->object_entity[i].compatible, index_head, (int)(index_tail - index_head));
			
			object_package_pair->object_entity[i].obj_type = drv_trigger_flip;
			object_package_pair->object_entity[i].obj_direct = object_direct;
		}
		object_package_pair->object_num = object_token_num;
	}
	
	return object_package_pair;
message_syntax_error:
	if(object_package_pair != NULL)
		util_free(object_package_pair);
	platform_debug("syntax error in message pair <%s>.\r\n", message_pair);
	return NULL;
}

static void message_probe_pair(const object_package_t *inst_object, const object_package_t *auxiliary_object)
{
	int i;
	char **compatible = NULL;
	char **auxiliary_compatible = NULL;
	
	if(!ASSERT_UTILS(inst_object) || !ASSERT_UTILS(auxiliary_object))
		return;
	
	for(i = 0; i < inst_object->object_num; i++)
		compatible = util_alloc(inst_object->object_num, char *);
	if(NULL == compatible)
		goto memory_alloc_fail;
	
	for(i = 0; i < auxiliary_object->object_num; i++)
		auxiliary_compatible = util_alloc(auxiliary_object->object_num, char *);
	if(NULL == auxiliary_compatible)
		goto memory_alloc_fail;
	
	for(i = 0; i < inst_object->object_num; i++) {
		compatible[i] = util_alloc(20, char);
		if(compatible[i] == NULL) {
			platform_error("memory alloc fail.\r\n");
			goto memory_alloc_fail;
		}
		strcpy(compatible[i], inst_object->object_entity[i].compatible);
		
		switch(inst_object->object_entity[i].obj_type) {
			case drv_trigger_high : strcat(compatible[i], "-TH"); break;
			case drv_trigger_low : strcat(compatible[i], "-TL"); break;
			case drv_trigger_flip : strcat(compatible[i], "-TF"); break;
			case drv_equals_high : strcat(compatible[i], "-EH"); break;
			case drv_equals_low : strcat(compatible[i], "-EL"); break;
			case drv_analog : strcat(compatible[i], "-A"); break;
			default: break;
		}
	}
	
	for(i = 0; i < auxiliary_object->object_num; i++) {
		auxiliary_compatible[i] = util_alloc(20, char);
		if(auxiliary_compatible[i] == NULL) {
			platform_error("memory alloc fail.\r\n");
			goto memory_alloc_fail;
		}
		
		strcpy(auxiliary_compatible[i], auxiliary_object->object_entity[i].compatible);
		
		switch(auxiliary_object->object_entity[i].obj_type) {
			case drv_trigger_high : strcat(auxiliary_compatible[i], "-TH"); break;
			case drv_trigger_low : strcat(auxiliary_compatible[i], "-TL"); break;
			case drv_trigger_flip : strcat(auxiliary_compatible[i], "-TF"); break;
			case drv_equals_high : strcat(auxiliary_compatible[i], "-EH"); break;
			case drv_equals_low : strcat(auxiliary_compatible[i], "-EL"); break;
			case drv_analog : strcat(auxiliary_compatible[i], "-A"); break;
			default: break;
		}

	}
	
	for(i = 0; i < inst_object->object_num; i++) {
		platform_bus_probe(	
					compatible[i], 
					inst_object->object_entity[i].obj_direct, 
					inst_object->object_entity[i].obj_type, 
					NULL, 
					auxiliary_object->object_num, 
					auxiliary_compatible, 
					inst_object->object_entity[i].extra_val 
				  );
		
	}
	
	for(i = 0; i < auxiliary_object->object_num; i++) {
		platform_bus_probe(	
					auxiliary_compatible[i], 
					auxiliary_object->object_entity[i].obj_direct, 
					auxiliary_object->object_entity[i].obj_type, 
					NULL, 
					inst_object->object_num, 
					compatible, 
					auxiliary_object->object_entity[i].extra_val 
				  );
		
	}
	
memory_alloc_fail:
	if(compatible != NULL) {
		for(i = 0; i < inst_object->object_num; i++) {
			if(compatible[i] != NULL)
				util_free(compatible[i]);
			else
				break;
		}
		util_free(compatible);
	}
	if(auxiliary_compatible != NULL) {
		for(i = 0; i < inst_object->object_num; i++) {
			if(auxiliary_compatible[i] != NULL)
				util_free(auxiliary_compatible[i]);
			else
				break;
		}
		util_free(auxiliary_compatible);
	}	
}

int message_helper(const char *raw_message_string)
{
	char message_string[200];
	int message_type;
	int message_cmd_type;
	int message_len;
	char *in_pair = NULL, *out_pair = NULL;
	object_package_t *init_object_package;
	object_package_t *in_object_package;
	object_package_t *out_object_package;
	
	
	if(NULL == raw_message_string)
		return -1;
	if((message_len = strlen(raw_message_string)) >= 200) {
		platform_debug("message <%s> too long.\r\n", raw_message_string);
		return -1;
	}
	
	memset(message_string, 0, sizeof(message_string));
	omit_extra_space(raw_message_string, message_string, message_len);
	
	message_type = message_get_type(message_string);
	if(MESSAGE_TYPE_ERR == message_type)
		return -1;
	
	if(MESSAGE_TYPE_NORMAL == message_type) {
		
		message_get_pair(message_string, "WHEN", &in_pair, &out_pair);
		in_object_package = message_handle_pair(in_pair, drv_in);
		message_cmd_type = message_get_cmd_type(out_pair);
		out_object_package = message_handle_pair(out_pair, drv_out);
		message_probe_pair(in_object_package, out_object_package);
		util_free(in_pair);
		util_free(out_pair);
		util_free(in_object_package);
		util_free(out_object_package);
	}
	else if(MESSAGE_TYPE_INIT == message_type) {
		init_object_package = message_handle_pair(message_string, drv_out);

	}
	
	
	
	return 0;
}

