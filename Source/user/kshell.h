#ifndef __KSHELL_H
#define __KSHELL_H

#include "stdio.h"
#include "common_utils.h"

#define __KSHELL_DEBUG

#ifdef __KSHELL_DEBUG
	#define __kshell_debug(fmt, arg...)           printf(""fmt"", ##arg)
#else
	#define __kshell_debug(fmt, arg...)
#endif

void __kshell_exec(char *cmd_buf, int size);

#define exec(cmd)	__kshell_exec(cmd, strlen(cmd));

#endif

