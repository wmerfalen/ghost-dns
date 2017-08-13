#ifndef __GHOST_CONF_HEADER__
#define __GHOST_CONF_HEADER__ 1
#include <cstring>
#include <cstdio>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unordered_map>

#define GDNS_RESOLVE_NONE 0
#define GDNS_RESOLVE_ALL 1
#define GDNS_RESOLVE_TRANSLATED 2
#define GDNS_RESOLVE_LOCALHOST 3
#define GDNS_RESOLVE_NO_TRANSLATION 4

#define GDNS_BUFFER_SIZE 512
/** FIXME: find the C++ way of doing this stupid macro */
#include <stdlib.h>
#define GDNS_DEBUG(...) fprintf(stderr,__VA_ARGS__);

struct translation{    
    char* key;                  
    char* value;               
    struct translation * next; 
};


namespace gdns {
	template <typename StringType>
	class conf {
/** Phase 2: convert these to private stl container member variables */
protected:
	/** FIXME: prefer unique ptrs to these pointers */
FILE* fp = NULL;
struct translation *list = NULL;
struct translation *head = NULL;
struct translation *settings = NULL;
struct translation *settings_head = NULL;
std::string m_config_file;
std::unordered_map<std::string,std::string> m_settings;
std::unordered_map<std::string,std::string> m_translations;

public:
conf(){
    fp = NULL;
    list = NULL;
}
conf(const std::string & config_file) : 
	m_config_file(config_file),
	fp(fopen(config_file.c_str(),"r")),
	list(nullptr) { }
~conf(){
	/** FIXME: prefer unique ptrs to these pointers */
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

bool exists(void){
#ifdef GHOSTDNS_CONFIG_FILE
    return (fp = fopen(GHOSTDNS_CONFIG_FILE,"r")) != NULL;
#else
    return (fp = fopen("/etc/ghost.conf","r")) != NULL;
#endif
}

int resolve(StringType node,StringType& target_host){
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

int parse(void){
#ifndef GDNS_STL
    char buffer[GDNS_BUFFER_SIZE];
    char *temp = NULL;
#else
	std::string temp;
#endif

    if(fp == NULL){
        if(!exists()){
            return -1;
        }
    }
    while(!feof(fp)){
		/** FIXME: prefer STL containers over these linked lists. For now it's not a huge
		 * hit since nobody is going to use this software except us (ROFL) but still haha
		 * do something about it damnit! --The Editor
		 */
#ifdef GDNS_STL
		std::array<char,GDNS_BUFFER_SIZE> buffer;
		std::fill(buffer.begin(),buffer.end(),0);
        temp = fgets((char*)&buffer[0],GDNS_BUFFER_SIZE,fp);
		if(temp.length() == 0){
			continue;
		}
		std::string key,value;
		if(m_parse_setting(temp,key,value)){
			m_settings.insert({key,value});
		}else if(m_parse_translation(temp,key,value)){
			m_translations.insert({key,value});
		}
#else
        memset(buffer,0,GDNS_BUFFER_SIZE);
        temp = fgets(buffer,GDNS_BUFFER_SIZE,fp);
        if(!temp){
            break;
        }
        std::string key,value;
        if(m_parse_setting(temp,key,value)){
            GDNS_DEBUG("[parse setting]: %s\n",temp);
            if(!settings){
                settings = (struct translation*)malloc(sizeof(struct translation));
                settings_head = settings; }else{
                settings->next = (struct translation*)malloc(sizeof(struct translation));
                settings = settings->next; }
            settings->key = key;
            settings->value = value;
            settings->next = NULL;
        }else if(m_parse_translation(temp,key,value)){
            GDNS_DEBUG("[parse translation]: %s\n",temp);
            struct translation * t = (struct translation*)malloc(sizeof(struct translation));
            if(!list){
                head = list = t; }else{
                list->next = t;
                list = list->next; }
            t->key = key;
            t->value = value;
            t->next = NULL; }
#endif
    }
    return 0;
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

int m_parse_translation(const StringType& line,StringType& out_key,StringType& out_value){
	auto f = line.find("=");
	if(f != std::string::npos){
		out_key = line.substr(0,f-1);
		out_value = line.substr(f+1,line.length() - f - 1);
		std::cout << out_key << "->(" << out_value << ")\n";
		return 1;
	}else{
		return 0;
	}

}

private:
int m_parse_setting(const StringType& line,StringType& out_key,StringType& out_value){
    auto f = line.find("!");
    if(strchr(line.c_str(),'!') && f != std::string::npos){
        std::string key = line.substr(0,f-1);
        std::string value = line.substr(f+1,line.length() - f - 1);
        if(key.length() == 0){
            out_key =  out_value = nullptr;
            return 0;
        }
        out_key = key;
        out_value = value;
        return 1;
    }else{
        return 0;
    }
}

int m_get_setting(const StringType& setting,StringType& out){
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


void my_free(void* ptr){
    GDNS_DEBUG("Freeing: '%s'...",ptr);
    free(ptr);
    GDNS_DEBUG("[freed]\n");
}

};	/* End class */

}; /* End namespace */
#endif
