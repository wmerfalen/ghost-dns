#ifndef __GHOST_CONF_HEADER__
#define __GHOST_CONF_HEADER__ 1
#include "util.hpp"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <stdio.h>
#include <string.h>
#include <memory>

#ifdef __GHOST_DNS_SHOW_DEBUG_OUTPUT__
#define GDNS_DEBUG(A) std::cerr << "[gdns::conf][debug]:'" << A << "'\n";
#else
#define GDNS_DEBUG(A)
#endif

namespace gdns {
	struct conf {
			conf() = delete;
			conf(const std::string& config_file) :
				m_config_file(config_file) {
				m_fp.open(config_file.c_str(),std::ios::in);
				GDNS_DEBUG("opened");
			}
			~conf() {
				if(m_fp.is_open()) {
					m_fp.close();
				}
			}

			bool exists() {
				GDNS_DEBUG("exists?");
				return (fopen(m_config_file.c_str(),"r")) != NULL;
			}


			int resolve(const std::string& node,std::string& target_host) {

				GDNS_DEBUG("setting to localhost");
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
				std::string all_setting;
				GDNS_DEBUG("checking all setting");
				if(m_get_setting("all",all_setting)) {
					GDNS_DEBUG("[ghost] all setting: '%s'\n" << all_setting);
					target_host = all_setting;
					GDNS_DEBUG("all setting present and set");
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
				GDNS_DEBUG("checking translations");
				for(auto t : m_translations) {
					GDNS_DEBUG("echoing first node");
					GDNS_DEBUG("[ghost] debug key: '" << t.first << "' node: '" << node << "'\n");
					/**
					 * TODO:
					 * It would be nice to have regular expression matches
					 */
					std::string temp = node;
					if(temp.compare(t.first) == 0) {
						GDNS_DEBUG("[ghost] found resolve translation: " << t.first);
						GDNS_DEBUG("[ghost] value: " << t.second);
						target_host = t.second;
						GDNS_DEBUG("translated return");
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
				GDNS_DEBUG("checking if localhost setting flag");
				if(m_get_setting_flag("localhost")) {
					GDNS_DEBUG("[ghost] settings flag !localhost set");
					target_host = "localhost";
					return GDNS_RESOLVE_LOCALHOST;
				}
				return GDNS_RESOLVE_NO_TRANSLATION;
			}

			int8_t parse() {
				GDNS_DEBUG("parse entry");
				std::string temp;

				if(!exists()) {
					GDNS_DEBUG("File doesn't exist");
					return -1;
				}
				if(!m_fp.is_open()) {
					m_fp.open(m_config_file.c_str(),std::ios::in);
				}
				if(!m_fp.is_open()) {
					return -2;
				}
				while(!m_fp.eof() && m_fp.good()) {
					/** FIXME: prefer STL containers over these linked lists. For now it's not a huge
					 * hit since nobody is going to use this software except us (ROFL) but still haha
					 * do something about it damnit! --The Editor
					 */
					std::array<char,GDNS_BUFFER_SIZE+1> buffer;
					std::fill(buffer.begin(),buffer.end(),0);
					m_fp.getline((char*)&buffer[0],GDNS_BUFFER_SIZE);
					temp = (char*)&buffer[0];
					util::trim(temp);
					if(temp.length() == 0) {
						continue;
					}
					std::string key,value;
					if(m_parse_setting(temp,key,value)) {
						GDNS_DEBUG("Parsed setting");
						GDNS_DEBUG(key.c_str());
						GDNS_DEBUG(value.c_str());
						m_settings.insert({key,value});
					} else if(m_parse_translation(temp,key,value)) {
						GDNS_DEBUG("Parsed translation");
						GDNS_DEBUG(key.c_str());
						GDNS_DEBUG(value.c_str());
						m_translations.insert({key,value});
					}
				}
				return 0;
			}


			void dump_list(void) { }

			int m_parse_translation(const std::string& line,std::string& out_key,std::string& out_value) {
				auto f = line.find("=");
				if(f != std::string::npos) {
					out_key = line.substr(0,f-1);
					out_value = line.substr(f+1,line.length() - f - 1);
					util::trim(out_key);
					util::trim(out_value);
					GDNS_DEBUG(out_key << "->" << out_value);
					return 1;
				} else {
					GDNS_DEBUG("translation not found");
					return 0;
				}

			}

		private:
			int m_parse_setting(const std::string& line,std::string& out_key,std::string& out_value) {
				auto f = line.find("!");
				if(strchr(line.c_str(),'!') && f != std::string::npos) {
					std::string key = line.substr(0,f-1);
					std::string value;
					if(f+1 > line.length() || (line.length() -f - 1) <= 0) {
						value = "";
					} else {
						value = line.substr(f+1,line.length() - f - 1);
					}
					util::trim(key);
					util::trim(value);
					if(key.length() == 0) {
						out_key =  out_value = nullptr;
						return 0;
					}
					out_key = key;
					out_value = value;
					return 1;
				} else {
					return 0;
				}
			}

			int m_get_setting(const std::string& setting,std::string& out) {
				GDNS_DEBUG("checking m_settings");
				for(auto t : m_settings) {
					GDNS_DEBUG("loop");
					GDNS_DEBUG("[ghost] setting: '" << setting << "', key: '" << t.first << "'");
					if(setting.compare(t.first.c_str()) == 0) {
						out = t.second;
						GDNS_DEBUG("found one. set it to: '" << t.second << "'");
						return 1;
					}
				}
				GDNS_DEBUG("out set to nullptr");
				out = "";
				return 0;
			}

			int m_get_setting_flag(const std::string& setting) {
				std::string ignored;
				return m_get_setting(setting,ignored);
			}


		private:
			std::ifstream m_fp;
			std::string m_config_file;
			std::unordered_map<std::string,std::string> m_settings;
			std::unordered_map<std::string,std::string> m_translations;

	};	/* End class */

}; /* End namespace */
#endif
#undef GDNS_DEBUG
