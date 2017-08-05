#include "../../util-conf.hpp"
#include <cstring>
#define TMP_FILE "/tmp/genstr.tmp"

int found = 0;
bool scan_flags(int argc,char** argv,const char* flag){
    int i;
    for(i=argc -1;strcmp(argv[i],flag) != 0 && i > 0;--i){}
    found = i;
    return i > 0;
}

int main(int argc,char** argv){
    using namespace mods::util::conf;
    if(argc < 2){
        std::cerr << "Usage: ./genstr [-stdout] <input.conf> [output.conf]\n";
        return 1;
    }
    if(scan_flags(argc,argv,"-stdout")){
        genstr(argv[found+1],TMP_FILE);
        FILE *fp = fopen(TMP_FILE,"r");
        while(!feof(fp)){
            std::array<char,1024> buf;
            std::fill(buf.begin(),buf.end(),0);
            fread((char*)&buf[0],1023,sizeof(char),fp);
            std::cout << (char*)&buf[0];
        }
        fclose(fp);
        return 0;
    }

    genstr(argv[1],argv[2]);
    return 0;
}
