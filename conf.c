#include "conf.h"

static FILE* fp = NULL;
static struct translation *list = NULL;
static struct translation *head = NULL;

int gdns_conf_exists(void){
    return (fp = fopen("/etc/ghost.conf","r")) != NULL;
}

#define GDNS_BUFFER_SIZE 512

int gdns_conf_parse(void){
    char *key;
    char *value;
    char buffer[GDNS_BUFFER_SIZE];
    char *temp = NULL;
    char *start = NULL;
    char *end = NULL;
    if(fp == NULL){
        if(!gdns_conf_exists()){
            return 0;
        }
    }
    while(!feof(fp)){
        memset(buffer,0,GDNS_BUFFER_SIZE);
        temp = fgets(buffer,GDNS_BUFFER_SIZE,fp);
        if(!temp){
            break;
        }
        temp = strtok(temp,"=");
        if(temp == NULL)
            continue;
        char* trimmed;
        trim_string(temp,&trimmed);
        if(!trimmed || strlen(trimmed) == 0)
            continue;
        key = trimmed;
        temp = strtok(NULL,"=");
        if(temp == NULL)
            continue;
        trim_string(temp,&trimmed);
        if(!trimmed || strlen(trimmed) == 0){
            if(key)
                free(key);
            continue;
        }
        value = trimmed;

        struct translation * t = (struct translation*)malloc(sizeof(struct translation));
        if(!list){
            list = t;
            head = list;
        }else{
            list->next = t;
        }
        t->key = key;
        t->value = value;
        t->next = NULL;
    }
    gdns_dump_list();
    return 1;
}

void gdns_dump_list(void){
    struct translation * t = head;
    do{
        printf("%s:%s\n",t->key,t->value);
    }while(t = t->next);
}

void trim_string(char* string,char** out){
    char* start = NULL;
    char* end = NULL;
    int i=0;
    for(start=string;start[i] && isspace(start[i]);i++);
    start += i;
    end = start;
    i = strlen(start)-1;
    for(;end[i] && isspace(end[i]);--i);
    end = &start[i+1];
    *out = (char*)malloc(end - start);
    strncpy(*out,start,end-start);
}

void gdns_init(void){
    fp = NULL;
    list = NULL;
}

void gdns_cleanup(void){
    struct translation * n = NULL;
    struct translation * temp = NULL;
    if(fp)
        fclose(fp);
    if(list){
        //TODO: free linked list
        n = list;
        while(n->next != NULL){
            free(n->key);
            free(n->value);
            temp = n;
            n = n->next;
            free(temp);
        }
    }
}
