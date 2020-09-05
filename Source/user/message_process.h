#ifndef __MESSAGE_PROCESS_H
#define __MESSAGE_PROCESS_H

#include "stm32f10x.h"
#include "platform_device.h"
#include "platform_bus.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

enum message_process_state {
	condition_insert = 1,
	result_insert,
	init_insert
};

typedef struct NODE
{
    char compatible[20];
    int drv_type;           // when in head ,it means the num of node
    int analog_quantity;
    struct NODE *link;
}*PNode;

extern PNode in_head;
extern PNode out_head;

extern char **in_compatible;
extern char **out_compatible;

//list process
void list_init(void);            //init in & out list

void message_list_add(  char *name,
                enum message_process_state sta,
                enum plat_driver_type type,
                int analog_quantity);


//个体解析
uint8_t individual_process( char *string,                    //
                            enum message_process_state sta,
                            uint8_t j,
                            uint8_t string_len
                            );


//条件解析
uint8_t content_process(char *string,                       
                        enum message_process_state sta,
                        uint8_t j,
                        uint8_t string_len
                        ); 

//语句解析
char string_process(const char *string);    

#endif
