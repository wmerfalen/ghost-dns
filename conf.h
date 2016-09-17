#ifndef __GHOST_CONF_HEADER__
#define __GHOST_CONF_HEADER__ 1
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/shm.h>

#define GDNS_RESOLVE_NONE 0
#define GDNS_RESOLVE_ALL 1
#define GDNS_RESOLVE_TRANSLATED 2
#define GDNS_RESOLVE_LOCALHOST 3
#define GDNS_RESOLVE_NO_TRANSLATION 4
#define GDNS_RESOLVE_NEEDS_FREE 4

#define GDNS_BUFFER_SIZE 512
#define GDNS_DEBUG(...) fprintf(stderr,__VA_ARGS__);

struct translation{    
    char* key;                  
    char* value;               
    struct translation * next; 
};


void gdns_init(void);
int gdns_resolve(char* node,char** target_host);
int gdns_conf_exists(void);
int gdns_conf_parse(void);
int gdns_get_translation(const char* node,char** translated);
void gdns_cleanup(void);
void trim_string(char*,char**);
void gdns_dump_list(void);
int gdns_conf_parse_setting(char*,char**,char**);
int gdns_conf_get_setting(char*,char**);
int gdns_conf_get_setting_flag(char*);
int gdns_conf_parse_translation(char*,char**,char**);

#endif
