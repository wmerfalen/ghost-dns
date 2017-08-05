#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <dlfcn.h>
#include "cpp/conf.hpp" 

extern "C" {
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



typedef hostent *(*orig_gethostbyname_f_type)(const char*name);
typedef int (*orig_gethostbyname_r_f_type)(const char*name,
        hostent* ret,char* buf, size_t buflen,
        hostent ** result,int *h_errnop
        );
typedef int (*orig_getaddrinfo_f_type)(const char* node,
        const char* service,const addrinfo *hints,
        addrinfo **res);
typedef void (*orig_exit_group_type)(int status);
typedef void (*orig_exit_type)(int status);
typedef ssize_t (*orig_send_type)(int,const void*,size_t,int);

void report_lookup(const char*,const char*);
static int init = 0;


void debug(const char* f){
	std::cerr << "[ghost-dns] debug: " << f << "\n";
}
int resolve_host(const std::string & node,std::string & resolved_ptr){
	debug("resolve_host");
    if(node.compare("0.0.0.0") == 0){
		debug("resolve_host -- 0.0.0.0");
        resolved_ptr = "0.0.0.0";
        return 0;
    }
    if(!init){
		debug("resolve_host -- initializing gdns");
        gdns_init();
        init = 1;
        if(gdns_conf_parse() == 0){
			debug("resolve_host -- found resolved ptr for node");
			debug(resolved_ptr.c_str());
            resolved_ptr = node;
            return 0;
        }
    }
    int ret = gdns_resolve<std::string,std::string>(node,resolved_ptr);
    switch(ret){
        case GDNS_RESOLVE_NO_TRANSLATION:
        case GDNS_RESOLVE_NONE: 
            resolved_ptr = node;
            break;
        case GDNS_RESOLVE_LOCALHOST:
            resolved_ptr = "localhost";
            break;
        default: break;
    }
    return ret;
}

hostent * gethostbyname(char* name){
    report_lookup(name,"gethostbyname");
    orig_gethostbyname_f_type orig;
    orig = (orig_gethostbyname_f_type)dlsym(RTLD_NEXT,"gethostbyname");   
    std::string resolved;
    resolve_host(name,resolved);
    hostent * h = orig(resolved.c_str());
    return h;
}

int gethostbyname_r(char* name,
        hostent* ret, char* buf, size_t buflen,
        hostent** result,int *h_errnop){
    report_lookup(name,"gethostbyname_r");
    orig_gethostbyname_r_f_type orig;
    orig = (orig_gethostbyname_r_f_type)dlsym(RTLD_NEXT,"gethostbyname_r");   
    std::string resolved;
    resolve_host(name,resolved);
    int i = orig(resolved.c_str(),ret,buf,buflen,result,h_errnop);
    return i;
}

int getaddrinfo(char* node,
        const char* service,const addrinfo *hints,
        addrinfo **res){
	const char * host = node;
    report_lookup(node,"getaddrinfo");
	if(service && (atoi(service) == 443 || atoi(service) == 80)){
		host = "localhost";
	}
    orig_getaddrinfo_f_type orig;
    orig = (orig_getaddrinfo_f_type)dlsym(RTLD_NEXT,"getaddrinfo");
    std::string resolved;
    resolve_host(node,resolved);
    int i = orig(resolved.c_str(),service,hints,res);
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



void report_lookup(const char *lookup,const char* function){
    fprintf(stderr,"[ghost] lookup: %s[%s]\n",lookup,function);
}

}//End extern "C"
