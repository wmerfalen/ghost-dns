#ifndef __GHOST_CONF_HEADER__
#define __GHOST_CONF_HEADER__ 1
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

struct translation{    
    char* key;                  
    char* value;               
    struct translation * next; 
};

void gdns_init(void);
int gdns_conf_exists(void);
int gdns_conf_parse(void);
int gdns_get_translation(const char* node,char** translated);
void gdns_cleanup(void);
void trim_string(char*,char**);
void gdns_dump_list(void);
#endif
