#ifndef __GHOST_DNS_SHM_HEADER__
#define __GHOST_DNS_SHM_HEADER__ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include <iostream>

#ifdef GHOSTDNS_USE_SHM_CONFIG
#include "shm-config.hpp"
#endif

extern int errno;

namespace gdns {
class  shm {
    public:
        shm();
        ~shm(){ if(m_data && m_id){ detach(); } }
        int alloc(const char*,int,size_t,int);
#ifdef GHOSTDNS_USE_SHM_CONFIG
		/** 
		 * param: size_t memory size to alloc
		 * param: int flags to send to shmget function
		 * return 0 if okay, less than zero on error
		 */
        inline int alloc(size_t s,int f){ return alloc(::shm::memory_id,::shm::project_id,s,f); }
		/**
		 * param size_t the size of the shared memory segment to grab
		 * param char** the char* pointer to place the data
		 * return 0 if okay, less than zero on error
		 */
        static inline int access(size_t memory_size,char** out_ptr){
            return access(::shm::memory_id,::shm::project_id,memory_size,out_ptr);
        }
		/**
		 * Automatically detect the size of the shared memory segment
		 */
        inline size_t detect_size(){
            return detect_size(::shm::memory_id,::shm::project_id);
        }
#endif
        size_t detect_size(const char*,int);
        static inline int access(const char*,int,size_t,char**);
        int info(struct shmid_ds*);
        bool ok(){ return m_data && m_id; }
        int detach();
        int destroy();
        char* ptr() const { return m_data; }
    private:
        bool m_attached;
        char* m_data;
        int m_id;
};


shm::shm(){
    m_attached = false;
    m_data = nullptr;
    m_id = 0;
}

size_t shm::detect_size(const char* memory_id,int project_id){
    struct shmid_ds buf;
    if(!m_attached){
        if(alloc(memory_id,project_id,1,IPC_CREAT | 0666) < 0){
            std::cerr << "shm::detect_size alloc error: " << strerror(errno) << "\n";
            return -1;
        }
    }
    if(info(&buf) < 0){
        std::cerr << "shm::detect_size info error: " << strerror(errno) << "\n";
        return -2;
    }
    return buf.shm_segsz;
}

int shm::info(struct shmid_ds* buf){
    if(shmctl(m_id,IPC_STAT,buf) < 0){
        std::cerr << "shm::info error: " << strerror(errno) << "\n";
        return -1;
    }
    return 0;
}

int shm::access(const char* memory_id,int project_id,size_t memory_size,char** out_ptr){
    key_t key = ftok(memory_id,project_id);
    if(key == -1){
        std::cerr << "access::ftok error\n";
        std::cerr << strerror(errno) << "\n";
        return -3;
    }
    std::cout << "access:key: " << key << "\n";
    int id;
    if ((id=shmget(key,memory_size , 0644 | IPC_CREAT | SHM_HUGETLB)) == -1) {
		std::cerr << "shmget error\n";
        std::cerr << strerror(errno);
		return -1;
    }
    *out_ptr = (char*)shmat(id,(void*)0,0);
    if(*out_ptr == (char*)(-1)){
        std::cerr << "access::shmat error\n";
        *out_ptr = nullptr;
        return -2;
    }
    return id;
}

int shm::alloc(const char* memory_id,int project_id,size_t memory_size,int flags){
    key_t key = ftok(memory_id,project_id);
    std::cout << "alloc:key: " << key << "\n";
    if(key == -1){
        std::cerr << "alloc::ftok error\n";
        std::cerr << strerror(errno) << "\n";
        return -3;
    }

    /*  create the segment: */
    if ((m_id = shmget(key, memory_size, flags)) == -1) {
		std::cerr << "shmget error\n";
        std::cerr << strerror(errno);
        m_data = nullptr;
		return -1;
    }

    /* attach to the segment to get a pointer to it: */
    m_data = (char*)shmat(m_id, (void *)0, 0);
    if (m_data == (char *)(-1)) {
		std::cerr << "shmat error\n";
        std::cerr << strerror(errno);
		m_data = nullptr;
		return -2;
    }
    return 0;
}


int shm::detach(){
    if(!m_attached){
        return -1;
    }
    /* detach from the segment: */
    if (shmdt((void*)m_data) == -1) {
		std::cerr << "shmdt error\n";
        std::cerr << strerror(errno);
		return -3;
	}
    m_attached = false;
	return 0;
}

int shm::destroy(){
    if(shmctl(m_id,IPC_RMID,nullptr) < 0){
        std::cerr << "destroy::shm_unlink error\n" << strerror(errno) << "\n";
        return -1;
    }
    m_data = nullptr;
    m_id = 0;
    return 0;
}

}; /* End namespace */

#endif

