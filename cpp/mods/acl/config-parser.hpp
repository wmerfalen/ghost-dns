#ifndef __CIRCLEMUD_SRC_MODS_ACL_PARSER_HEADER__
#define __CIRCLEMUD_SRC_MODS_ACL_PARSER_HEADER__ 1

#include <iostream>
#include <fstream>
#include <string>
#include <stack>
#include <regex>
#include <tuple>
#include <map>
#include <algorithm>
//#include "radix_tree/radix_tree.hpp"
#include "color.hpp"
#include "../util-map.hpp"
#include <cstdarg>  //for variadic function m_expect_chain(int ...)
#include <assert.h>

#define dbg(f) if(print_debug){  std::cout << Color::fg::green <<\
    "[" << Color::bg::blue << "debug: ]" << Color::bg::def << \
     Color::fg::blue << f << "\n" << Color::fg::def;\
    }
#define dbg_grn(f) if(print_debug){ grn_text(f); }
#define dbg_blu(f) if(print_debug){ grn_text(f); }
#define dbg_red(f) if(print_debug){ grn_text(f); }

#define dm(m,prefix) if(print_debug){\
        for(auto i: m){ std::cout << prefix << i << "\n"; }\
    }
#define SKIP_WHITESPACE() \
    while(!m_out_of_bounds() && isspace(m_at())){\
        dbg("m_at SKIP");\
        if(m_at() == '\n'){ m_line_number++; }\
        m_increment_file_offset(1);\
    }


namespace mods {
    namespace acl {
        class FileParser {

            public:
                /****************/
                /* Public types */
                /****************/
                /*
                 * The following examples are based off of a situation where we parse this example conf:
                 *
                 * -playable
                 *      allow -> 
                 *          files : [
                 *              act.wizard.c
                 *              act.offensive.c
                 *          ]
                 *      deny ->
                 *          files : [
                 *              main.c
                 *              foobar.c
                 *          ]
                 *
                 * We will assign a value to m_command_map as such:
                 *
                 * m_command_map["act.wizard.c"] = 1;
                 * m_command_map["act.offensive.c"] = 2; 
                 * m_command_map["main.c"] = 3;
                 * m_command_map["foobar.c"] = 4;
                 *
                 * rule playable;
                 * playable[m_command_map["act.wizard.c"]] = 1; //Allow act.wizard.c 
                 * playable[m_command_map["act.offensive.c"]] = 1; //Allow act.offensive.c 
                 * playable[m_command_map["main.c"]] = 0; //Deny main.c 
                 * playable[m_command_map["foobar.c"]] = 0; //Deny foobar.c 
                 *
                 * m_tree["playable"] = playable;
                 *
                 * ==================
                 * = Finding a rule =
                 * ==================
                 * bool FileParser::allowed(const char* role,const char* file_or_command){
                 *  return m_tree[role][m_command_map[file_or_command]];
                 * }
                 *
                 *
                 */
                typedef std::vector<bool>  rule;
                typedef std::string tr_key;
                typedef rule tr_value;
                typedef std::map<tr_key,tr_value> tree;
                typedef std::string cm_key;
                typedef uint64_t cm_value;
                typedef std::map<cm_key,cm_value> command_map;
                typedef uint64_t int_type;
                enum PARSE_TYPE { E_CLASS, E_EXTENDED_CLASS, E_COMMENT, E_BLOCK,    \
                    E_ACCESS_TYPE, E_ACCESS_START, E_SCOPE, E_LIST_START, E_STRING, \
                    E_ALLOW, E_DENY,\
                    E_LIST_END, E_COLON, E_EXTENDS, E_ARROW, E_FILES, E_COMMANDS,   \
                    E_DEFAULT\
                };
                enum ITEM_TYPE { ITEM_TYPE_FILES, ITEM_TYPE_COMMANDS };
                /* Constants */
                static const int PARSE_FAIL;
                static const int FILE_CANNOT_OPEN;
                static const int FILE_EOF;
                FileParser() : print_debug(false),m_command_ctr(0),m_fp(nullptr),\
                               m_opened(false),m_file_offset(0),m_tentative_file_offset(0),\
                               m_line_number(1),m_dont_advance_file_offset(false){}
                ~FileParser();
                FileParser(const std::string &);
                void setFile(const char* f){m_file_name = f; }
                const char* getFile() const { return m_file_name.c_str(); }
                int read();
                int parse();
                void dump_tree(void);
                inline void dump_rules(rule r);
                bool print_debug;
            private:
                int m_accept_regex(const std::string &,std::vector<std::string>&);
                int m_accept_regex(const std::string &);
                int m_move_after_regex(const char*);
                int m_expect(PARSE_TYPE);
                int m_expect_chain(int,...);
                template <typename T>
                int m_expect_chain(const T &,std::vector<int>&);
                template <typename  T>
                T m_expect(PARSE_TYPE);
                int m_comment(void);
                int m_block(void);
                int m_access_rules(void);
                int m_arrow(void);
                int m_class(void);
                int m_extended_class(void);
                int m_default(void);
                std::tuple<int,bool> m_access_type(void);
                int m_colon(void);
                int m_extends(void);
                void m_access_start(void);
                int m_files(void);
                int m_commands(void);
                void m_scope(void);
                int m_list_start(void);
                int m_list_end(void);
                void m_string(void);

                int m_store_items(const std::string&);
                int m_store_extends(std::string&);

                std::vector<std::string> m_parse_list(void);


                /* Commands and rules */
                command_map m_command_map;
                cm_value m_max_command;
                tree m_tree;
                std::string m_current_class;
                std::string m_current_extended_class;
                cm_value m_command_ctr;
                PARSE_TYPE m_current_access_type;
                inline void m_register_commands(const std::vector<std::string>& vec,PARSE_TYPE type);
                inline void m_save_base_class(const std::vector<std::string>& vec);
                inline void m_save_extended_class(const std::vector<std::string>& vec,const std::string & base_class);
                inline void m_save_extended_class_default(const std::string & base_class);
                command_map m_d_map;

                /* File properties */
                std::string m_file_name;
                std::ifstream m_fp;
                std::string m_file_contents;
                bool m_opened;
                size_t m_file_offset;
                size_t m_tentative_file_offset;
                int_type m_line_number;
                bool m_dont_advance_file_offset;

                inline bool m_out_of_bounds(void);
                void m_nextline(void);
                char m_next_char(void);
                int m_move_after(char);
                size_t m_get_file_offset(void);
                size_t m_increment_file_offset(size_t);
                inline void m_set_tentative_file_offset(size_t i);
                inline void m_toggle_file_offset_advance(bool b);

                /* Status and reporting functions */
                void m_report_line(const char* s);

                /* Utility functions */
                std::string m_enum_to_string(int);
                inline char m_at(size_t offset); 
                inline char m_at(void);
                inline std::string m_substr(void);
                inline void m_advance(size_t i);
                inline bool m_still_have_content();
                /* Purely utility functions for development */
                inline void util_print_until(char c); 
                inline void m20();

        };

    };

};


#endif
