#include "parse_utils.h"
#include "kshell.h"
#include "platform_event.h"
#include "platform_bus.h"
#include "message_process.h"

void (*terminal_handler)(const char *);

static void __kshell_import(const char *import_option);
static void __kshell_ls(const char *ls_option);
static void __kshell_clean(void);

static void path_del_tail(char *path)
{
	char *p = path + strlen(path) - 1;
	while(*p != '/' && p > path) {
		--p;
	}
	if(p == path)
		return;
	else
		memset(p, 0, strlen(p));
}

static void omit_extra_space(const char *raw_cmd, char *new_cmd, int size)
{
	int i, j;
	char temp[size];
	
	if(size <= 0)
		return;
	
	memset(temp, 0, size);
	
	/*掐头*/
	for(i = 0; i < size; i++) {
		if(raw_cmd[i] != ' ')
			break;
	}
	
	/*去尾*/
	for(j = size - 1; j >= 0; j--) {
		if(raw_cmd[j] != ' ')
			break;
	}
	strncpy(temp, raw_cmd + i, j - i + 1);
	
	j = 1;
	/*再去掉中间连续的空格*/
	for(i = 1; i < size; i++) {
		if(!(temp[i] == ' ' && temp[i - 1] == ' '))
			new_cmd[j++] = temp[i];
	}
	new_cmd[0] = temp[0];
}

void __kshell_exec(char *cmd_buf, int size)
{
	int i;
	uint32_t offset = 0;
	int argc = 0;
	char cmd_exec[size + 1];
	char cmd[10];
	char *argv[10];
	uint8_t *p = NULL;
	
	if(0 == size)
		return;
	__kshell_debug("\r\n> ");
	if(!strcmp(cmd_buf, "esc")) {
		terminal_handler = NULL;
		return;
	}
	if(terminal_handler == NULL) {
		memset(cmd, 0, 10);
		memset(cmd_exec, 0, size + 1);
		
		omit_extra_space(cmd_buf, cmd_exec, size);
		ParseLoadPacket((uint8_t *)cmd_exec, size);
		if(NULL == (p = ParseGetNLabel((uint8_t *)" ", 1, &offset))) {
			offset = strlen(cmd_exec);
			memcpy(cmd, cmd_exec, (offset > 9) ? 9 : offset);
		}
		else
			strncpy(cmd, cmd_exec, (offset > 9) ? 9 : offset);
		
		p_packet.bp = p_packet.bp - 1;
		while(NULL != (p = ParseGetNContent((uint8_t *)" ", (uint8_t *)" ", 1, &offset))) {
			argv[argc] = util_alloc(offset + 1, char);
			memset(argv[argc], 0, offset + 1) ;
			memcpy(argv[argc], p, offset);
			++argc;
		}
		if(*p_packet.bp != 0) {
			offset = strlen((char *)p_packet.bp);
			argv[argc] = util_alloc(offset, char);
			memset(argv[argc], 0, offset);
			memcpy(argv[argc], p_packet.bp + 1, offset - 1);
			++argc;
		}
		
		if(!strcmp(cmd, "import")) {
			if(argc == 1)
				__kshell_import(argv[0]);
			else
				__kshell_debug("\"import\": argument mismatch, requires 1.");
		}
		else if(!strcmp(cmd, "ls")) {
			if(argc == 1)
				__kshell_ls(argv[0]);
			else
				__kshell_debug("\"ls\": argument mismatch, requires 1.");
		}
		else if(!strcmp(cmd, "clean")) {
			if(argc == 0)
				__kshell_clean();
			else
				__kshell_debug("\"clean\": argument mismatch, requires 0.");
		}
		else
			__kshell_debug("unknown command \"%s\"", cmd);
		
		for(i = 0; i < argc; i++) {
			util_free(argv[i]);
		}
	}
	else
		terminal_handler(cmd_buf);
}

static void platform_device_import_handler(const char *message)
{
	platform_device_parse_configuration(message);
	platform_device_print_list(device_list);
}

static void platform_driver_import_handler(const char *message)
{
	if(0 == string_process(message)) {
		platform_bus_match_list(IN_EVENT_LIST_ID, OUT_EVENT_LIST_ID);
		__kshell_debug("driver logic file import done.\r\n");
		__kshell_debug("> event generated.");
		init_flag = 1;
		platform_event_print_list(INIT_EVENT_LIST_ID);
		platform_event_print_list(IN_EVENT_LIST_ID);
		platform_event_print_list(OUT_EVENT_LIST_ID);
		terminal_handler = NULL;
	}
}

static void platform_checkclean_handler(const char *message)
{
	if(!(strcmp(message, "yes"))) {
		__kshell_debug("clean done.");
		terminal_handler = NULL;
	}
	else if(!(strcmp(message, "no"))) {
		__kshell_debug("clean canceled.");
		terminal_handler = NULL;
	}
	else
		__kshell_debug("unknown option, input [yes/no].");
}


static void __kshell_import(const char *import_option)
{
	if(!strcmp(import_option, "-dev")) {
		__kshell_debug("input device descriptive file:");
		terminal_handler = platform_device_import_handler;
	}
	else if(!strcmp(import_option, "-drv")) {
		__kshell_debug("input driver logic file:");
		terminal_handler = platform_driver_import_handler;
	}
	else
		__kshell_debug("\"import\": unknown argument \"%s\".", import_option);
}

static void __kshell_ls(const char *ls_option)
{
	if(!strcmp(ls_option, "-dev")) {
		platform_device_print_list(device_list);
	}
	else if(!strcmp(ls_option, "-ev")) {
		platform_event_print_list(INIT_EVENT_LIST_ID);
		platform_event_print_list(IN_EVENT_LIST_ID);
		platform_event_print_list(OUT_EVENT_LIST_ID);
	}
	else
		__kshell_debug("\"ls\": unknown argument \"%s\".", ls_option);
}

static void __kshell_clean()
{
	__kshell_debug("clean all infomation? [yes/no].");
	terminal_handler = platform_checkclean_handler;
}
