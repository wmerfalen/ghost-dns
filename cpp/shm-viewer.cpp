#define GHOSTDNS_USE_SHM_CONFIG
#include "shm.hpp"
#include "shm-config.hpp"
#include <stdlib.h>
#include <unistd.h>
#include <memory>

using namespace gdns;

void usage(const char* prog){
    std::cerr << "Usage: " << prog << " [-d (dump memory)] [-s (dump size)]\n";
}

int main(int argc,char** argv){
    gdns::shm s;
    int opt;
    bool f_get_size = false;
    bool f_dump_memory = false;

    if(argc == 1){
        usage(argv[0]);
        return 0;
    }

    while ((opt = getopt(argc, argv, "sd")) != -1) {
       switch (opt) {
       case 's':
           f_get_size = true;
           break;
       case 'd':
           f_dump_memory= true;
           break;
       default: /* '?' */
           std::cerr << "Unknown parameter: " << opt << "\n";
           usage(argv[0]);
           return 0;
       }
    }

    if(s.alloc(atoi(argv[1]),IPC_CREAT | 0666) < 0){
        std::cerr << "Cannot alloc shm\n";
        return -1;
    }
    auto data = std::make_unique<char*>(s.ptr());

    struct shmid_ds buf;
    if(s.info(&buf) < 0){
        std::cerr << "Cannot get shm info\n";
        return -2;
    }

    if(f_get_size){	
        std::cout << "[shm size]: " << s.detect_size() << "\n";
    }

    if(f_dump_memory){
        size_t size = s.detect_size();
        if(size < 0){
            std::cerr << "[dump memory] error: Invalid size detected\n";
            return -1;
        }
        for(size_t i=0; i < size;i++){
            printf("%x",(*data)[i]);
        }
    }
    s.detach();
    return 0;
}
