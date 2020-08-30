#include <stdio.h>
#include "platform_device.h"
#include "parson.h"

plat_device_list_t device_list;

static GPIO_TypeDef *translate_gpio_string(const char *gpio_string)
{
	if(!strcmp(gpio_string, "GPIOA"))
		return GPIOA;
	else if(!strcmp(gpio_string, "GPIOB"))
		return GPIOB;
	if(!strcmp(gpio_string, "GPIOC"))
		return GPIOC;
	else if(!strcmp(gpio_string, "GPIOD"))
		return GPIOD;
	if(!strcmp(gpio_string, "GPIOE"))
		return GPIOE;
	else if(!strcmp(gpio_string, "GPIOF"))
		return GPIOF;
	else if(!strcmp(gpio_string, "GPIOG"))
		return GPIOG;
	else
		return NULL;
}

plat_device_entity_t *platform_device_get_entity_v_compatible(const plat_device_list_t *device_list, const char *dev_compatible)
{
	int i;
	struct list_head *t = NULL;
        plat_device_entity_t *p = NULL;
	
	if(!ASSERT_UTILS(device_list))
		return NULL;
	
	list_for_each(t, device_list->device_head) {
		p = list_entry(t, plat_device_entity_t);
		if(!strcmp(p->dev_compatible, dev_compatible))
			return p;
	}
	return NULL;
}

plat_device_attr_t *platform_device_get_board_attr_v_compatible(const plat_device_list_t *device_list, const char *dev_compatible)
{
	int i;
	struct list_head *t = NULL;
	
        plat_device_entity_t *p = platform_device_get_entity_v_compatible(device_list, dev_compatible);
	if(NULL == p)
		return NULL;
	
	return &(p->dev_board_attr);
}

int platform_device_construct_list(plat_device_list_t *device_list, int list_id)
{
	if(!ASSERT_UTILS(device_list))
		return -1;
	LIST_CREATE_UTILS(device_list->device_head, device_list);
	if(!ASSERT_UTILS(device_list->device_head))
		return -1;
	list_head_init(device_list->device_head);
	device_list->dev_list_id = list_id;
	device_list->device_num = 0;
	
	return 0;
}

int platform_device_register(plat_device_list_t *device_list, const plat_device_entity_t *pNewdeviceEntity)
{
	if(!ASSERT_UTILS(device_list))
		return -1;
	DEVICE_CREATE_UTILS(new_entity, plat_device_entity_t);
	if(!ASSERT_UTILS(new_entity))
		return -1;
	
	memcpy(new_entity, pNewdeviceEntity, sizeof(plat_device_entity_t));
	new_entity->dev_id = device_list->device_num;
	LIST_CREATE_UTILS(new_entity->dev_entry, new_entity);
	if(!ASSERT_UTILS(new_entity)) {
		util_free(new_entity);
		return NULL;
	}
	list_add_tail(new_entity->dev_entry, device_list->device_head);
	
	++device_list->device_num;
	return 0; 
}

char *platform_device_generate_emu_configuration()
{
	char *s = NULL;
	
	JSON_Value *pJsonRoot = json_value_init_object();
	JSON_Value *pJsonSub = json_value_init_object();
	
	
	JSON_Value *pJsonDeviceArray = json_value_init_array();
	json_object_set_value(json_object(pJsonRoot), "device", pJsonDeviceArray);
	
	pJsonSub = json_value_init_object();
	json_array_append_value(json_array(pJsonDeviceArray), pJsonSub);
	json_object_set_string(json_object(pJsonSub), "compatible", "CON1");
	json_object_set_string(json_object(pJsonSub), "GPIO", "GPIOA");
	json_object_set_number(json_object(pJsonSub), "GPIO_Pin", 1);
	json_object_set_number(json_object(pJsonRoot), "device_num", 1);
	
	s = json_serialize_to_string_pretty(pJsonRoot);
	json_value_free(pJsonRoot);
	printf("%s\r\n", s);
	return s;
}

void platform_device_parse_configuration(const char *conf_json_string)
{
	int i, j;
	plat_device_entity_t new_device;
	memset(&new_device, 0, sizeof(plat_device_entity_t));
	
	JSON_Value *pJsonRoot = json_parse_string(conf_json_string);
	int dev_number = json_object_get_number(json_object(pJsonRoot), "device_num");
	JSON_Array *pJsonArray = json_object_get_array(json_object(pJsonRoot), "device");
	if(NULL != pJsonArray) {
		for(i = 0; i < dev_number; i++) {
			JSON_Object *pJsonSub = json_array_get_object(pJsonArray, i);
			
			const char *compatible = json_object_get_string(pJsonSub, "compatible");
			if(NULL != compatible)
				memcpy(new_device.dev_compatible, compatible, strlen(compatible));
			
			const char *gpio_string = json_object_get_string(pJsonSub, "GPIO");
			if(NULL != gpio_string) {
				new_device.dev_board_attr.GPIO = translate_gpio_string(gpio_string);
				new_device.dev_board_attr.GPIO_Pin = json_object_get_number(pJsonSub, "GPIO_Pin");
			}
			platform_device_register(&device_list, &new_device);
		}
	}
parse_fail:
	json_value_free(pJsonRoot);
}

void platform_device_print_list(const plat_device_list_t *device_list)
{
	int i;
	struct list_head *t = NULL;
        plat_device_entity_t *p = NULL;
	
	if(!ASSERT_UTILS(device_list))
		return;
	
	printf("\r\ndevice list id:%d\r\n", device_list->dev_list_id);
	printf("list entity number:%d\r\n", device_list->device_num);
	list_for_each(t, device_list->device_head) {
		printf("------------------------------------------------\r\n");
		p = list_entry(t, plat_device_entity_t);
		printf("\t<compatible>: %s\r\n", p->dev_compatible);
		printf("\t<device id>:%d\r\n", p->dev_id);
		printf("\t<device board attribution>:\r\n");
		switch((uint32_t)p->dev_board_attr.GPIO) {
			case GPIOA_BASE: printf("\t\t<GPIO>: GPIOA\r\n"); break;
			case GPIOB_BASE: printf("\t\t<GPIO>: GPIOA\r\n"); break;
			case GPIOC_BASE: printf("\t\t<GPIO>: GPIOA\r\n"); break;
			case GPIOD_BASE: printf("\t\t<GPIO>: GPIOA\r\n"); break;
			case GPIOE_BASE: printf("\t\t<GPIO>: GPIOA\r\n"); break;
			case GPIOF_BASE: printf("\t\t<GPIO>: GPIOA\r\n"); break;
			default: printf("\t\t<GPIO>: unknown\r\n"); break;
		}
		printf("\t\t<GPIO_Pin>: %d\r\n", p->dev_board_attr.GPIO_Pin); 
	}
}
