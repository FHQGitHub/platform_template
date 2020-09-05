#include "platform_kobject.h"

plat_kobject_list_t *kobject_list;

plat_kobject_t *platform_kobject_create()
{
	CREATE_UTILS(new_kobject, plat_kobject_t);
	if(!ASSERT_UTILS(new_kobject))
		return NULL;
	
	util_memset(new_kobject, 0, sizeof(plat_kobject_t));
	LIST_CREATE_UTILS(new_kobject->kobj_entry, new_kobject);
	if(!ASSERT_UTILS(new_kobject->kobj_entry)) {
		util_free(new_kobject);
		return NULL;
	}
	return new_kobject;
}

void platform_kobject_destroy()
{
	
}

plat_kobject_list_t *platform_kobject_construct_list(int kobject_list_id)
{
	CREATE_UTILS(kobject_list, plat_kobject_list_t);
	if(!ASSERT_UTILS(kobject_list))
		return NULL;
	
	LIST_CREATE_UTILS(kobject_list->kobj_head, kobject_list);
	if(!ASSERT_UTILS(kobject_list->kobj_head)) {
		util_free(kobject_list);
		return NULL;
	}
	list_head_init(kobject_list->kobj_head);
	kobject_list->kobj_list_id = kobject_list_id;
	kobject_list->kobj_num = 0;
	
	return kobject_list;
}

plat_kobject_list_t *platform_kobject_destruct_list(int kobject_list_id)
{
	
}

void platform_kobject_register(plat_kobject_list_t *kobject_list, plat_kobject_t *new_kobject)
{
	
	if(!ASSERT_UTILS(kobject_list) || !ASSERT_UTILS(new_kobject))
		return;
	
	list_add_tail(new_kobject->kobj_entry, kobject_list->kobj_head);
	++kobject_list->kobj_num;
}

plat_kobject_t *platform_kobject_get_entity_v_compatible(const plat_kobject_list_t *kobject_list, const char *kobj_compatible)
{
	struct list_head *t = NULL;
        plat_kobject_t *p = NULL;

	if(!ASSERT_UTILS(kobject_list))
		return NULL;
	
	list_for_each(t, kobject_list->kobj_head) {
		p = list_entry(t, plat_kobject_t);
		if(!strcmp(p->compatible, kobj_compatible))
			return p;
	}
	return NULL;
}

static uint32_t get_gpio_clock(GPIO_TypeDef *GPIO)
{
	if(GPIO==GPIOA)
		return RCC_APB2Periph_GPIOA;
	else if(GPIO==GPIOB)
		return RCC_APB2Periph_GPIOB;
	if(GPIO==GPIOC)
		return RCC_APB2Periph_GPIOC;
	else if(GPIO==GPIOD)
		return RCC_APB2Periph_GPIOD;
	if(GPIO==GPIOE)
		return RCC_APB2Periph_GPIOE;
	else if(GPIO==GPIOF)
		return RCC_APB2Periph_GPIOF;
	else if(GPIO==GPIOG)
		return RCC_APB2Periph_GPIOG;
	else
		return 0;
}

static uint8_t get_gpio_mode(enum plat_driver_direct gpio_direct,enum plat_driver_type gpio_type)
{
	if(gpio_direct==drv_in)
	{
		if(gpio_type==drv_analog)
		{
			return GPIO_Mode_AIN;
		}else if(gpio_type==drv_trigger_high)

		{
			return GPIO_Mode_IPD;
		}else if(gpio_type==drv_trigger_low)
		{
			return GPIO_Mode_IPU;
		}
	}else if (gpio_direct==drv_out)
	{
		return GPIO_Mode_Out_PP;
	}
}

void platform_kobject_board_init(const kobject_kernel_t *kobject_kernel)
{
	uint32_t clock=0;
	uint8_t mode=0;
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	clock = get_gpio_clock(kobject_kernel->dev_kernel->dev_board_attr.GPIO);
	mode = get_gpio_mode(kobject_kernel->drv_kernel->drv_direct, kobject_kernel->drv_kernel->drv_type);

	RCC_APB2PeriphClockCmd(clock,ENABLE);

	GPIO_InitStructure.GPIO_Pin=(1 << kobject_kernel->dev_kernel->dev_board_attr.GPIO_Pin);
	GPIO_InitStructure.GPIO_Mode=mode;
 	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz;	
	GPIO_Init(kobject_kernel->dev_kernel->dev_board_attr.GPIO, &GPIO_InitStructure);					 
}

uint16_t platform_kobject_board_read(const kobject_kernel_t *kobject_kernel)
{
	uint16_t value;
	if(kobject_kernel->drv_kernel->drv_type != drv_analog) {
		platform_kobject_board_init(kobject_kernel);
		value=GPIO_ReadInputDataBit(kobject_kernel->dev_kernel->dev_board_attr.GPIO,1 << kobject_kernel->dev_kernel->dev_board_attr.GPIO_Pin);
	}
	else if (kobject_kernel->drv_kernel->drv_type == drv_analog) {
		/* analog process */
	}
	
	return value; //to return a digital value<0:1> or an analog value<0:4096>
}

void platform_kobject_board_write(const kobject_kernel_t *kobject_kernel, int val)
{
	if(kobject_kernel->drv_kernel->drv_type != drv_analog) {
		if(0 == val)
			GPIO_WriteBit(kobject_kernel->dev_kernel->dev_board_attr.GPIO, 1 << kobject_kernel->dev_kernel->dev_board_attr.GPIO_Pin, Bit_RESET);
		else if(1 == val)
			GPIO_WriteBit(kobject_kernel->dev_kernel->dev_board_attr.GPIO, 1 << kobject_kernel->dev_kernel->dev_board_attr.GPIO_Pin, Bit_SET);
	}
	else if(kobject_kernel->drv_kernel->drv_type != drv_analog) {
		/* analog process */
	}
}

void platform_kobject_get_hw_status(const plat_kobject_list_t *kobject_list)
{
	struct list_head *t = NULL;
	plat_kobject_t *p = NULL;
	
	if(NULL == kobject_list)
		return;
		
	list_for_each(t, kobject_list->kobj_head) {
		p = list_entry(t, plat_kobject_t);
		if(p->kernel.drv_kernel->drv_direct == drv_in)
			p->kernel.dev_kernel->dev_in_value[0] = platform_kobject_board_read(&(p->kernel));
	}
}


