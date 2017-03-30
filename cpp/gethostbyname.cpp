#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include "conf.hpp" 
#include "shm-config.hpp"

namespace gdns {
typedef struct _hostent {       \
    char    *h_name;            \
    char    **h_aliases;        \
    int     h_addrtype;         \
    int     h_length;           \
    char**  h_addr_list;        \
} hostent;

typedef unsigned int uint32_t;
typedef uint32_t socklen_t;

typedef struct _addrinfo {
   int              ai_flags;		\
   int              ai_family;		\
   int              ai_socktype;	\
   int              ai_protocol;	\
   socklen_t        ai_addrlen;		\
   struct sockaddr *ai_addr;		\
   char            *ai_canonname;	\
   struct _addrinfo *ai_next;		\
} addrinfo;


};

typedef gdns::hostent *(*orig_gethostbyname_f_type)(const char*name);
typedef int (*orig_gethostbyname_r_f_type)(const char*name,
        struct hostent* ret,char* buf, size_t buflen,
        struct hostent ** result,int *h_errnop
        );
typedef int (*orig_getaddrinfo_f_type)(const char* node,
        const char* service,const struct addrinfo *hints,
        struct addrinfo **res);
typedef void (*orig_exit_group_type)(int status);
typedef void (*orig_exit_type)(int status);
typedef ssize_t (*orig_send_type)(int,const void*,size_t,int);



void report_lookup(const char*,const char*);
static int init = 0;

int resolve_host(const char* node,char** resolved_ptr){
    if(strcmp(node,"0.0.0.0") == 0){
        *resolved_ptr = "0.0.0.0";
        return 0;
    }
    if(!init){
        gdns_init();
        init = 1;
        if(gdns_conf_parse() == 0){
            *resolved_ptr = const_cast<char*>(node);
            return 0;
        }
    }
    char* ptr = NULL;
    char* resolved = NULL;
    int ret = gdns_resolve(const_cast<char*>(node),&resolved);
    switch(ret){
        case GDNS_RESOLVE_NO_TRANSLATION:
        case GDNS_RESOLVE_NONE: 
            resolved = const_cast<char*>(node);
            break;
        case GDNS_RESOLVE_LOCALHOST:
            resolved = "localhost";
            break;
        default: break;
    }
    ptr = (char*)malloc(strlen(resolved)+1);
    memset(ptr,0,strlen(resolved)+1);
    strncpy(ptr,resolved,strlen(resolved));
    *resolved_ptr = ptr;
    return ret |= 1 << GDNS_RESOLVE_NEEDS_FREE;
}

extern "C" {
gdns::hostent * gethostbyname(const char* name){
    report_lookup(name,"gethostbyname");
    orig_gethostbyname_f_type orig;
    orig = (orig_gethostbyname_f_type)dlsym(RTLD_NEXT,"gethostbyname");   
    char* resolved = NULL;
    int ret = resolve_host(name,&resolved);
    gdns::hostent * h = orig(resolved);
    if(ret & GDNS_RESOLVE_NEEDS_FREE){
        free(resolved);
    }
    return h;
}

int gethostbyname_r(const char* name,
        struct hostent* ret, char* buf, size_t buflen,
        struct hostent** result,int *h_errnop){
    report_lookup(name,"gethostbyname_r");
    orig_gethostbyname_r_f_type orig;
    orig = (orig_gethostbyname_r_f_type)dlsym(RTLD_NEXT,"gethostbyname_r");   
    char* resolved = NULL;
    int rh_ret = resolve_host(name,&resolved);
    int i = orig(resolved,ret,buf,buflen,result,h_errnop);
    if(rh_ret & GDNS_RESOLVE_NEEDS_FREE){
        free(resolved);
    }
    return i;
}

int getaddrinfo(const char* node,
        const char* service,const struct addrinfo *hints,
        struct addrinfo **res){
	const char * host = node;
    report_lookup(node,"getaddrinfo");
	if(service && (atoi(service) == 443 || atoi(service) == 80)){
		host = "localhost";
	}
    orig_getaddrinfo_f_type orig;
    orig = (orig_getaddrinfo_f_type)dlsym(RTLD_NEXT,"getaddrinfo");
    char * resolved = NULL;
    int ret = resolve_host(node,&resolved);
    int i = orig(resolved,service,hints,res);
    if(ret & GDNS_RESOLVE_NEEDS_FREE){
        free(resolved);
    }
    return i;
}

void exit_group(int status){
    fprintf(stderr,"[ghost] exit_group: %d\n",status);
    gdns_cleanup();
    orig_exit_group_type orig;
    orig = (orig_exit_group_type)dlsym(RTLD_NEXT,"exit_group");
    orig(status);   
}

void exit(int status){
    fprintf(stderr,"[ghost] exit: %d\n",status);
    gdns_cleanup();
    orig_exit_type orig;
    orig = (orig_exit_type)dlsym(RTLD_NEXT,"exit");
    orig(status);
}

ssize_t send(int sockfd,const void* buf,size_t len,int flags){
    orig_send_type orig = (orig_send_type)dlsym(RTLD_NEXT,"send");
    ssize_t ret = orig(sockfd,buf,len,flags);
    return ret;
}
} /* End extern "C" */

void report_lookup(const char *lookup,const char* function){
    fprintf(stderr,"[ghost] lookup: %s[%s]\n",lookup,function);
}
