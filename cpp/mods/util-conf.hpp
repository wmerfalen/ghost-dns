#ifndef __MENTOC_UTIL_CONF_HEADER__
#define  __MENTOC_UTIL_CONF_HEADER__

#include <iostream>
#include <string>
#include <fstream>
#include <array>
#include <vector>

namespace mods {
    namespace util {
        namespace text {
            std::vector<std::string> split(const std::string & in_string,const std::string & delimiter,unsigned int limit = 1024){
                unsigned int ctr = limit;
                std::string::size_type start_pos = 0,end_pos = 0;
                std::vector<std::string> parts;
                do{
                    end_pos = in_string.find(delimiter,start_pos);
                    if(end_pos == std::string::npos){
                        if(start_pos < in_string.length()){
                            parts.push_back(in_string.substr(start_pos));
                        }
                        break;
                    }
                    parts.push_back(in_string.substr(start_pos,end_pos - start_pos));
                    start_pos = end_pos + delimiter.length();
                }while(ctr-- > 0 && start_pos < in_string.length());
                return parts;
            }
        };
        namespace conf {
            static bool stfu = true;
            char to_hex_digit(unsigned int);
            std::string tohex(char c){
                unsigned int i = (unsigned int)c;
                unsigned int left_over = i % 16;
                unsigned int char_one = i / 16;
                std::string chars;
                if(c < 0x10){
                    chars[0] = '0';
                }else{
                    chars[0] = to_hex_digit(char_one);
                }
                chars[1] = to_hex_digit(left_over);
                chars[2] = '\0';
                return chars;
            }
            char to_hex_digit(unsigned int target){
                if(target < 0x0A){
                    return (char)(target + 48);
                }
                if(target < 0x10){
                    return (char)(target + 55);
                }
                return '\0';
            }
            bool genstr(const std::string & file_name,
                    const std::string & out_file,
                    const char* var_name = "genstr",
                    const char* _name_space = "mods::util::conf::genstr"
                    ){
                std::ifstream m(file_name,std::ios::in | std::ios::binary);
                if(!m.is_open()){
                    if(::mods::util::conf::stfu == false){
                        std::cerr << "Couldn't open: " << file_name << "\n";
                    }
                    return false;
                }
                std::ofstream out(out_file,std::ios::trunc | std::ios::out);
                if(!out.is_open()){
                    if(::mods::util::conf::stfu == false){
                        std::cerr << "Couldn't open out file: " << out_file << "\n";
                    }
                    if(m.is_open()){
                        m.close();
                    }
                    return false;
                }
                out << "#ifndef __GENSTR_CONF_FILE__" << var_name << "__\n";
                out << "#define __GENSTR_CONF_FILE__" << var_name << "__\n";
                out << "#include <iostream>\n";
                unsigned n_space_ctr = 0;
                for(const auto & nm_space : ::mods::util::text::split(_name_space,"::",8)){
                    for(unsigned i = n_space_ctr; i > 0; i--){ out << "\t"; }
                    out << "namespace " << nm_space << "{\n";            //open the namespaces
                    n_space_ctr++;
                }
                out << "const unsigned char " << var_name << "[] = {";
                if(m.is_open()){
                    std::cout << "foo";
                    std::array<char,1024> buf;
                    buf[1023] = '\0';
                    unsigned ctr = 0;
                    do{
                        m.read((char*)&buf[0],1023);
                        while(buf[ctr] != '\0'){
                            if(ctr % 30 == 0){
                                out << "\n";
                                for(unsigned tabs=1; tabs < n_space_ctr;tabs++){
                                    out << "\t";
                                }
                            }
                            out << "0x" << ::mods::util::conf::tohex(buf[ctr++]).c_str();
                            if(buf[ctr] != '\0'){
                                out << ",";
                            }
                        }
                        std::fill(buf.begin(),buf.end(),0);
                    }while(!m.eof());
                    if(ctr){
                        out << ",";
                    }
                    out << "0x00";
                }
                out << "};\n";  //End the char*[] declaration
                for(unsigned i=0; i < n_space_ctr;i++){ 
                    for(unsigned tabs=1; tabs < n_space_ctr - i;tabs++){
                        out << "\t";
                    }
                    out << "};\n"; } //Close the namespaces
                out << "#endif\n"; //close the preproccessor
                m.close();
                out.close();
                return true;
            }
        };
    };

};

#endif
