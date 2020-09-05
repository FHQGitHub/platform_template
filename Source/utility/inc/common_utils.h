//
//  common_utils.h
//  ML
//
//  Created by Kios on 2020/2/27.
//  Copyright © 2020 Supersure. All rights reserved.
//

#ifndef common_utils_h
#define common_utils_h

#include <stdlib.h>
#include <string.h>
#include "malloc.h"

typedef  unsigned char 	uint8_t;
typedef  unsigned int  	uint32_t;

#define __MUST_FREE		

#define CREATE_UTILS(name, type)    	type *name = util_alloc(1, type)
#define ASSERT_UTILS(ptr)       	(ptr != NULL)

#define util_mem_init()			my_mem_init(0)
#define util_alloc(num, type)   	mymalloc(0, num * sizeof(type))
#define util_realloc(num, ptr, type)	myrealloc(0, ptr, num * sizeof(type))
#define util_memcpy(des, src, n)	mymemcpy(des, src, n)
#define util_memset(s, c, count)	mymemset(s, c, count)
#define util_free(p)            	myfree(0, p)

#define container_of(ptr, type, member) \
        ((type*)(((unsigned long)ptr) - (unsigned long)(&(((type*)0)->member))))

#endif /* common_utils_h */
