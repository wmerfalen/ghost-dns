#define GHOSTDNS_USE_SHM_CONFIG
#include "shm.hpp"
#include "shm-config.hpp"
#include <unistd.h>
#include <memory>
using namespace gdns;

int main(int argc,char** argv){
    gdns::shm s;
    if(argc != 2){
        std::cerr << "Use: " << argv[0] << "<shm_size>\n";
        return 0;
    }
    if(s.alloc(atoi(argv[1]),IPC_CREAT | 0666) < 0){
        std::cerr << "Cannot alloc shm\n";
        return -1;
    }
    auto data = std::make_unique<char*>(s.ptr());
    strncpy(*data,"lmao",4);
    while(1){ usleep(600); }
    s.detach();
    return 0;
}
