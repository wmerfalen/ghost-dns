#include "conf.hpp"

namespace gdns {


/** Phase 2: convert these to private stl container member variables */
static FILE* fp = NULL;
static struct translation *list = NULL;
static struct translation *head = NULL;
static struct translation *settings = NULL;
static struct translation *settings_head = NULL;

bool conf::exists(void){
#ifdef GHOSTDNS_CONFIG_FILE
    return (fp = fopen(GHOSTDNS_CONFIG_FILE,"r")) != NULL;
#else
    return (fp = fopen("/etc/ghost.conf","r")) != NULL;
#endif
}

template <typename StringType>
int conf<StringType>::resolve(StringType node,StringType& target_host){
    target_host = "localhost";
    struct translation *t = head;
    if(!head){
        return GDNS_RESOLVE_NONE;
    }

    /**************************************************************************/
    /*                         "All" feature                                  */
    /*========================================================================*/
    /* Let's say you don't necessarily trust an application. You can use the  */
    /* "all" setting to redirect all DNS calls to one IP address.             */
    /*                                                                        */
    /* Example usage:                                                         */
    /* !all = 127.0.0.1                                                       */
    /**************************************************************************/ 
    char* all_setting = NULL;
    if(m_get_setting("all",&all_setting)){
        GDNS_DEBUG("[ghost] all setting: '%s'\n",all_setting);
        target_host = all_setting;
        return GDNS_RESOLVE_ALL;
    }

    /**************************************************************************/
    /*                         Host Translations                              */
    /*========================================================================*/
    /* You can translate host names from strings to IP addresses using this   */
    /* feature. Simply specify a host name and assign that the desired IP     */
    /* address.                                                               */
    /*                                                                        */
    /* Example usage:                                                         */
    /* google.com = 127.0.0.1                                                 */
    /**************************************************************************/
    do{
        GDNS_DEBUG("[ghost] debug key: '%s' node: '%s'\n",t->key,node);
        //TODO: do regular expression match here
		std::string temp = node;
        if(temp.compare(t->key) == 0){
            GDNS_DEBUG("[ghost] found resolve translation: '%s'\n",t->key);
            GDNS_DEBUG("[ghost] value: '%s'\n",t->value);
            target_host = t->value;
            return GDNS_RESOLVE_TRANSLATED;
        }
    }while(t = t->next);

    /**************************************************************************/
    /*                        localhost flag                                  */
    /*========================================================================*/
    /* The localhost flag is a shortcut for doing !all = 127.0.0.1            */
    /*                                                                        */
    /* Example usage:                                                         */
    /* !localhost                                                             */
    /**************************************************************************/
    if(m_get_setting_flag("localhost")){
        GDNS_DEBUG("[ghost] settings flag !localhost set\n");
        target_host = "localhost";
        return GDNS_RESOLVE_LOCALHOST;
    }
    return GDNS_RESOLVE_NO_TRANSLATION;
}

template <typename StringType>
int conf<StringType>::parse(void){
    char buffer[GDNS_BUFFER_SIZE];
    char *temp = NULL;
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
        char* key,*value;
        if(m_parse_setting(temp,&key,&value)){
            GDNS_DEBUG("[parse setting]: %s\n",temp);
            if(!settings){
                settings = (struct translation*)malloc(sizeof(struct translation));
                settings_head = settings;
            }else{
                settings->next = (struct translation*)malloc(sizeof(struct translation));
                settings = settings->next;
            }
            settings->key = key;
            settings->value = value;
            settings->next = NULL;
        }else if(gdns_conf_parse_translation(temp,&key,&value)){
            GDNS_DEBUG("[parse translation]: %s\n",temp);
            struct translation * t = (struct translation*)malloc(sizeof(struct translation));
            if(!list){
                head = list = t;
            }else{
                list->next = t;
                list = list->next;
            }
            t->key = key;
            t->value = value;
            t->next = NULL;
        }
    }
    return 1;
}


void gdns_dump_list(void){
    struct translation * t = head;
    if(!head){
       return;
    }
    do{
        printf("%s:%s\n",t->key,t->value);
    }while(t = t->next);

    if(!settings_head){
        return;
    }
    t = settings_head;
    do{
        printf("%s:%s\n",t->key,t->value);
    }while(t = t->next);
}

template <typename StringType>
int conf<StringType>::m_parse_translation(const StringType& line,StringType& out_key,StringType& out_value){
    char *temp = strtok(line,"=");
    if(temp == NULL){
        out_key = out_value = nullptr;
        return 0;
    }

    char* trimmed;
    trim_string(temp,&trimmed);
    if(!trimmed || strlen(trimmed) == 0){
        out_key = out_value = nullptr;
        return 0;
    }
    out_key = trimmed;
    temp = strtok(NULL,"=");
    if(temp == NULL){
        out_value = nullptr;
        return 0;
    }
    trim_string(temp,&trimmed);
    if(!trimmed || strlen(trimmed) == 0){
        return 0;
    }
    out_value = trimmed;
    return 1;
}

template <typename StringType>
int conf<StringType>::m_parse_setting(const StringType& line,StringType& out_key,StringType& out_value){
    char* parts = NULL;
    if(strchr(line,'!') && ((parts = strtok(line,"!")) != NULL)){
        char* key = strtok(parts,"=");
        char* value = strtok(NULL,"=");
        if(key == NULL){
            out_key =  out_value = nullptr;
            return 0;
        }
        trim_string(key,&key);
        trim_string(value,&value);
        out_key = key;
        out_value = value;
        return 1;
    }else{
        return 0;
    }
}

template <typename StringType>
int conf<StringType>::m_get_setting(const StringType& setting,StringType& out){
    struct translation* t = settings_head;
    while(t){
        GDNS_DEBUG("[ghost] setting: '%s', key: '%s'\n",setting,t->key);
        if(strcmp(setting,t->key) == 0){
            out = t->value;
            return 1;
        }
        t = t->next;
    }
    *out = NULL;
    return 0;
}

template <typename StringType>
int m_get_setting_flag(const StringType& setting){
    struct translation* t = settings_head;
    while(t){
        GDNS_DEBUG("[ghost] setting: '%s', key: '%s'\n",setting,t->key);
        if(strcmp(setting,t->key) == 0){
            return 1;
        }
        t = t->next;
    }
    return 0;
}

void trim_string(char* string,char** out){
    char* start = NULL;
    char* end = NULL;
    if(!string || strlen(string) == 0){
        *out = NULL;
        return;
    }
    int i=0;
    for(start=string;start[i] && isspace(start[i]);i++);
    start += i;
    end = start;
    i = strlen(start)-1;
    for(;end[i] && isspace(end[i]);--i);
    end = &start[i+1];
    if(start == end || strlen(start) == 0){ 
        *out = NULL;
        return;
    }
    *out = (char*)malloc((end - start) + 1);
    memset(*out,0,(end -start) + 1);
    strncpy(*out,start,end-start);
}

template <typename StringType>
void conf<StringType>::init(void){
    fp = NULL;
    list = NULL;
}

void my_free(void* ptr){
    GDNS_DEBUG("Freeing: '%s'...",ptr);
    free(ptr);
    GDNS_DEBUG("[freed]\n");
}

template <typename StringType>
void conf<StringType>::~conf(void){
    struct translation * n = NULL;
    struct translation * temp = NULL;
    if(fp)
        fclose(fp);
    if(list){
        n = list;
        do{
            if(n->key){
                my_free(n->key);
            }
            if(n->value)
                my_free(n->value);
            temp = n;
            GDNS_DEBUG("Freeing temp...");
            free(temp);
            GDNS_DEBUG("[done]\n");
        }while(n = n->next);
        list = head = NULL;
    }

    if(settings_head){
        n = settings_head;
        do{
            if(n->key){
                my_free(n->key);
            }
            if(n->value){
                my_free(n->value);
            }
            temp = n;
            GDNS_DEBUG("Freeing settings head...");
            free(temp);
            GDNS_DEBUG("[done]\n");
        }while(n = n->next);
        settings = settings_head = NULL;
    }
}
};
