#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <dlfcn.h>
#define GDNS_STL
#include "conf.hpp"
/** We use this macro to make it easier to interface with the shm stuff */
#define GHOSTDNS_USE_SHM_CONFIG
#include "shm.hpp"

extern "C" {
	typedef struct _hostent {
		\
		char    *h_name;
		\
		char    **h_aliases;
		\
		int     h_addrtype;
		\
		int     h_length;
		\
		char**  h_addr_list;
		\
	} hostent;

	typedef unsigned int uint32_t;
	typedef uint32_t socklen_t;

	typedef struct _addrinfo {
		int              ai_flags;
		\
		int              ai_family;
		\
		int              ai_socktype;
		\
		int              ai_protocol;
		\
		socklen_t        ai_addrlen;
		\
		struct sockaddr *ai_addr;
		\
		char            *ai_canonname;
		\
		struct _addrinfo *ai_next;
		\
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

	std::shared_ptr<gdns::conf> conf() {
		static std::shared_ptr<gdns::conf> c;
		static bool initialized = false;
		if(!initialized) {
			c = std::make_shared<gdns::conf>("/etc/ghost.conf");
			initialized = true;
		}
		return c;
	}

	void debug(const char* f) {
#ifdef __GHOST_DNS_SHOW_DEBUG_OUTPUT__
		std::cerr << "[ghost-dns] debug: " << f << "\n";
#endif
	}
	int resolve_host(const std::string& node,std::string& resolved_ptr) {
		debug("resolve_host");
		if(node.compare("0.0.0.0") == 0) {
			debug("resolve_host -- 0.0.0.0");
			resolved_ptr = "0.0.0.0";
			return 0;
		}
		/** access the parsed configuration and resolve the host accordingly */
		return conf()->resolve(node,resolved_ptr);
	}

	hostent * gethostbyname(char* name) {
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
	                    hostent** result,int *h_errnop) {
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
	                addrinfo **res) {
		report_lookup(node,"getaddrinfo");
#if 0
		/** wtf is this??? */
		if(service && (atoi(service) == 443 || atoi(service) == 80)) {
			host = "localhost";
		}
#endif
		orig_getaddrinfo_f_type orig;
		orig = (orig_getaddrinfo_f_type)dlsym(RTLD_NEXT,"getaddrinfo");
		std::string resolved;
		resolve_host(node,resolved);
		int i = orig(resolved.c_str(),service,hints,res);
		return i;
	}

	ssize_t send(int sockfd,const void* buf,size_t len,int flags) {
		orig_send_type orig = (orig_send_type)dlsym(RTLD_NEXT,"send");
		ssize_t ret = orig(sockfd,buf,len,flags);
		return ret;
	}



	void report_lookup(const char *lookup,const char* function) {
		fprintf(stderr,"[ghost] lookup: %s[%s]\n",lookup,function);
	}

}//End extern "C"
