#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
 
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
   struct addrinfo *ai_next;		\
} addrinfo;



typedef struct hostent *(*orig_gethostbyname_f_type)(const char*name);
typedef int (*orig_gethostbyname_r_f_type)(const char*name,
        struct hostent* ret,char* buf, size_t buflen,
        struct hostent ** result,int *h_errnop
        );
typedef int (*orig_getaddrinfo_f_type)(const char* node,
        const char* service,const struct addrinfo *hints,
        struct addrinfo **res);

struct hostent * gethostbyname(const char* name){
    orig_gethostbyname_f_type orig;
    orig = (orig_gethostbyname_f_type)dlsym(RTLD_NEXT,"gethostbyname");   
    return orig("localhost");
}

int gethostbyname_r(const char* name,
        struct hostent* ret, char* buf, size_t buflen,
        struct hostent** result,int *h_errnop){
    orig_gethostbyname_r_f_type orig;
    orig = (orig_gethostbyname_r_f_type)dlsym(RTLD_NEXT,"gethostbyname_r");   
    return orig("localhost",ret,buf,buflen,result,h_errnop);
}

int getaddrinfo(const char* node,
        const char* service,const struct addrinfo *hints,
        struct addrinfo **res){
	const char * host = node;
	if(service && (atoi(service) == 443 || atoi(service) == 80)){
		host = "localhost";
	}
    orig_getaddrinfo_f_type orig;
    orig = (orig_getaddrinfo_f_type)dlsym(RTLD_NEXT,"getaddrinfo");
    return orig(host,service,hints,res);
}
