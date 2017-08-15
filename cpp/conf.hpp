#ifndef __GHOST_CONF_HEADER__
#define __GHOST_CONF_HEADER__ 1

#include "db.hpp"

namespace gdns {
	template <typename StringType>
	struct conf {
	private:
	std::ifstream m_fp;
std::string m_config_file;
std::unordered_map<std::string,std::string> m_settings;
std::unordered_map<std::string,std::string> m_translations;
	public: 
conf(){
}
conf(const std::string & config_file) : 
	m_config_file(config_file) {
		m_fp.open(config_file.c_str(),std::ios::in);
	}
~conf(){
	/** FIXME: prefer unique ptrs to these pointers */
    if(m_fp.is_open())
        m_fp.close();
}

bool exists(void){
#ifdef GHOSTDNS_CONFIG_FILE
    return (fopen(GHOSTDNS_CONFIG_FILE,"r")) != NULL;
#else
    return (fopen("/etc/ghost.conf","r")) != NULL;
#endif
}


int resolve(StringType node,StringType& target_host){

    target_host = "localhost";
    /**************************************************************************/
    /*                         "All" feature                                  */
    /*========================================================================*/
    /* Let's say you don't necessarily trust an application. You can use the  */
    /* "all" setting to redirect all DNS calls to one IP address.             */
    /*                                                                        */
    /* Example usage:                                                         */
    /* !all = 127.0.0.1                                                       */
    /**************************************************************************/ 
    char* all_setting = NULL;
    if(m_get_setting("all",&all_setting)){
        GDNS_DEBUG("[ghost] all setting: '%s'\n" << all_setting);
        target_host = all_setting;
        return GDNS_RESOLVE_ALL;
    }

    /**************************************************************************/
    /*                         Host Translations                              */
    /*========================================================================*/
    /* You can translate host names from strings to IP addresses using this   */
    /* feature. Simply specify a host name and assign that the desired IP     */
    /* address.                                                               */
    /*                                                                        */
    /* Example usage:                                                         */
    /* google.com = 127.0.0.1                                                 */
    /**************************************************************************/
	for(auto t : m_translations){
        GDNS_DEBUG("[ghost] debug key: '" << t.first() << "' node: '" << node << "'\n");
        /**
		 * TODO: 
		 * It would be nice to have regular expression matches
		 */
		std::string temp = node;
        if(temp.compare(t.first()) == 0){
            GDNS_DEBUG("[ghost] found resolve translation: " << t.first());
            GDNS_DEBUG("[ghost] value: " << t.second());
            target_host = t.second();
            return GDNS_RESOLVE_TRANSLATED;
        }
    }

    /**************************************************************************/
    /*                        localhost flag                                  */
    /*========================================================================*/
    /* The localhost flag is a shortcut for doing !all = 127.0.0.1            */
    /*                                                                        */
    /* Example usage:                                                         */
    /* !localhost                                                             */
    /**************************************************************************/
    if(m_get_setting_flag("localhost")){
        GDNS_DEBUG("[ghost] settings flag !localhost set");
        target_host = "localhost";
        return GDNS_RESOLVE_LOCALHOST;
    }
    return GDNS_RESOLVE_NO_TRANSLATION;
}

int parse(void){
#ifndef GDNS_STL
    char buffer[GDNS_BUFFER_SIZE];
    char *temp = NULL;
#else
	std::string temp;
#endif

	if(!exists()){
		GDNS_DEBUG("File doesn't exist");
		return -1;
	}
	if(!m_fp.is_open()){
		#ifdef GHOSTDNS_CONFIG_FILE
		m_fp.open(GHOSTDNS_CONFIG_FILE,std::ios::in);
		#else
		m_fp.open("/etc/ghost.conf",std::ios::in);
		#endif
	}
	if(!m_fp.is_open()){
		std::cerr << "[ghost] unable to open config file:"  << 
		#ifdef GHOSTDNS_CONFIG_FILE
		GHOSTDNS_CONFIG_FILE
		#else
		"/etc/ghost.conf"
		#endif
		<< "\n";
		return -2;
	}
    while(!m_fp.eof() && m_fp.good()){
		/** FIXME: prefer STL containers over these linked lists. For now it's not a huge
		 * hit since nobody is going to use this software except us (ROFL) but still haha
		 * do something about it damnit! --The Editor
		 */
		std::array<char,GDNS_BUFFER_SIZE> buffer;
		std::fill(buffer.begin(),buffer.end(),0);
        m_fp.getline((char*)&buffer[0],GDNS_BUFFER_SIZE);
		temp = (char*)&buffer[0];
		util::trim(temp);
		if(temp.length() == 0){
			continue;
		}
		std::string key,value;
		if(m_parse_setting(temp,key,value)){
			GDNS_DEBUG("Parsed setting");
			GDNS_DEBUG(key.c_str());
			GDNS_DEBUG(value.c_str());
			m_settings.insert({key,value});
		}else if(m_parse_translation(temp,key,value)){
			GDNS_DEBUG("Parsed translation");
			GDNS_DEBUG(key.c_str());
			GDNS_DEBUG(value.c_str());
			m_translations.insert({key,value});
		}
    }
    return 0;
}


void dump_list(void){ }

int m_parse_translation(const StringType& line,StringType& out_key,StringType& out_value){
	auto f = line.find("=");
	if(f != std::string::npos){
		out_key = line.substr(0,f-1);
		out_value = line.substr(f+1,line.length() - f - 1);
		util::trim(out_key);
		util::trim(out_value);
		std::cout << out_key << "->(" << out_value << ")\n";
		return 1;
	}else{
		GDNS_DEBUG("translation not found");
		return 0;
	}

}

private:
int m_parse_setting(const StringType& line,StringType& out_key,StringType& out_value){
    auto f = line.find("!");
    if(strchr(line.c_str(),'!') && f != std::string::npos){
        std::string key = line.substr(0,f-1);
        std::string value;
		if(f+1 > line.length() || (line.length() -f - 1) <= 0){
			value = "";
		}else{
			value = line.substr(f+1,line.length() - f - 1);
		}
		util::trim(key);
		util::trim(value);
        if(key.length() == 0){
            out_key =  out_value = nullptr;
            return 0;
        }
        out_key = key;
        out_value = value;
        return 1;
    }else{
        return 0;
    }
}

int m_get_setting(const StringType& setting,StringType& out){
	for(auto t : m_settings){
        GDNS_DEBUG("[ghost] setting: '" << setting << "', key: '" << t.first() << "'");
        if(setting.compare(t.first()) == 0){
            out = t.second();
            return 1;
        }
    }
    out = nullptr;
    return 0;
}

int m_get_setting_flag(const StringType& setting){
	std::string ignored;
	return m_get_setting(setting,ignored);
}

void my_free(void* ptr){
    GDNS_DEBUG("Freeing: '" << ptr << "'");
    free(ptr);
    GDNS_DEBUG("[freed]");
}


};	/* End class */

}; /* End namespace */
#endif
