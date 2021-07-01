#ifndef __GHOST_DB_HEADER__
#define __GHOST_DB_HEADER__
#include <cstring>
#include <cstdio>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include "liblmdb/lmdb.h"
#include <memory>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unordered_map>

#define GDNS_RESOLVE_NONE 0
#define GDNS_RESOLVE_ALL 1
#define GDNS_RESOLVE_TRANSLATED 2
#define GDNS_RESOLVE_LOCALHOST 3
#define GDNS_RESOLVE_NO_TRANSLATION 4
/** For our trim functions (stolen from: https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring ) */
#include <algorithm>
#include <cctype>
#include <locale>

#define GDNS_BUFFER_SIZE 512
/** FIXME: find the C++ way of doing this stupid macro */
#include <stdlib.h>

namespace gdns {
	struct util {
		static inline void ltrim(std::string& s) {
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
				return !std::isspace(ch);
			}));
		}

		// trim from end (in place)
		static inline void rtrim(std::string& s) {
			s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
				return !std::isspace(ch);
			}).base(), s.end());
		}

		// trim from both ends (in place)
		static inline void trim(std::string& s) {
			ltrim(s);
			rtrim(s);
		}

		// trim from start (copying)
		static inline std::string ltrim_copy(std::string s) {
			ltrim(s);
			return s;
		}

		// trim from end (copying)
		static inline std::string rtrim_copy(std::string s) {
			rtrim(s);
			return s;
		}

	};

	/** Client class */
	namespace lmdb {
		struct gdns_env {
			MDB_env* env;
			gdns_env(MDB_env* e) : env(e) {}
			gdns_env() : env(nullptr) {}
			~gdns_env() {
				if(env) {
					mdb_env_close(env);
				}
			}
		};
		struct gdns_txn {
			MDB_txn *txn;
			gdns_txn(MDB_txn* t) : txn(t) {}
			gdns_txn() : txn(nullptr) {}
			~gdns_txn() = default;
		};
		struct gdns_dbi {
			MDB_dbi dbi; 	/* this is just an unsigned int */
			gdns_dbi(MDB_dbi d) : dbi(d) {}
			~gdns_dbi() = default;
		};
		struct db {
				db() : m_good(false) {}
				~db() = default;
				db(const char* file, const char* dbi_name,int flags,int permissions,bool b_create,unsigned int max_dbs =1) {
					m_good = create(file,dbi_name,flags,permissions,b_create,max_dbs) == 0;
				}
				std::unique_ptr<gdns_env> env;
				std::unique_ptr<gdns_txn> txn;
				std::unique_ptr<gdns_dbi> dbi;
				int create(const char* file,const char* dbi_name,int flags,int permissions,bool b_create,unsigned int max_dbs) {
					/** Create lmdb handle */
					MDB_env *_env;
					if(mdb_env_create(&_env) < 0) {
						std::cerr << "[lmdb] failed to open new environment\n";
						return -1;
					}
					env = std::make_unique<gdns_env>(_env);

					/** Open the lmdb database handle */
					int open_flags = flags;
					if(dbi_name != NULL && b_create) {
						open_flags |= MDB_CREATE;
						mdb_env_set_maxdbs(env->env,max_dbs);
					}
					if(mdb_env_open(env->env,file,open_flags /*MDB_WRITEMAP | MDB_NOLOCK*/,permissions) < 0) {
						std::cerr << "[lmdb] failed to open directory '/tmp/ghostdns' make sure it exists!\n";
						return -1;
					}

					MDB_txn *_txn;
					{
						/** Begin transaction */
						if(mdb_txn_begin(env->env,NULL,flags /*MDB_WRITEMAP | MDB_NOLOCK*/,&_txn) < 0) {
							std::cerr << "[lmdb] failed to open transaction!\n";
							return -1;
						}
						txn = std::make_unique<gdns_txn>(_txn);

						/** Open the database */
						MDB_dbi _dbi;
						int ret = 0;
						int open_type = MDB_CREATE;
						if(!b_create) {
							open_type = MDB_RDONLY;
						}
						if((ret = mdb_dbi_open(txn->txn,dbi_name,open_type,&_dbi)) < 0) {
							std::cerr << "[lmdb] failed to open dbi connection\n";
							std::cerr << "[code]:" << ret << "\n";
							return -1;
						}
						dbi = std::make_unique<gdns_dbi>(_dbi);
					}
					return 0;
				}
				inline bool good() const {
					return m_good;
				}
				int get(const std::string& key,std::string& in_value) {
					if(m_good) {
						MDB_val k;
						k.mv_size = key.length();
						k.mv_data = (void*)key.c_str();
						MDB_val v;
						int ret = mdb_get(txn->txn,dbi->dbi,&k,&v);
						switch(ret) {
							case MDB_NOTFOUND:
								return 0;
							case EINVAL:
								std::cerr << "[lmdb] invalid parameter to mdb_get\n";
								return -1;
							default:
								in_value = static_cast<const char*>(v.mv_data);
								return 1;
						}
					}
					return -2;
				}
				int put(const std::string& key,const std::string& value) {
					if(m_good) {
						MDB_val k;
						k.mv_size = key.length();
						k.mv_data = (void*)key.c_str();
						MDB_val v;
						v.mv_size = value.length();
						v.mv_data = (void*)value.c_str();
						int ret = mdb_put(txn->txn,dbi->dbi,&k,&v,0);
						switch(ret) {
							case MDB_MAP_FULL:
								std::cerr << "[lmdb] database is full, see mdb_env_set_mapsize()\n";
								return -3;
							case EINVAL:
								std::cerr << "[lmdb] invalid parameter to mdb_get\n";
								return -1;
							case EACCES:
								std::cerr << "[lmdb] invalid parameter to mdb_get\n";
								return -4;
							case MDB_TXN_FULL:
								std::cerr << "[lmdb] transaction has too many dirty pages\n";
								return -5;
							default:
								return 1;
						}
					}
					return -2;
				}
			private:
				bool m_good;
		};
		struct client {
				client() = delete;
				~client() {
					m_good = false;
					if(m_db) {
						m_db.release();
					}
				}

				client(const std::string& file,const std::string&  db_name) {
					init(file,db_name);
				}

				bool init(const std::string& file,const std::string& db_name) {
					m_good = false;
					m_db = std::make_unique<db>(file.c_str(),db_name.c_str(),MDB_RDONLY,0333,false);
					m_good = m_db->good();
					return m_good;
				}
				bool good() const {
					return m_db->good();
				}
				int get(const std::string& key,std::string& value) {
					return m_db->get(key,value);
				}
				int put(const std::string& key,const std::string& value) {
					return m_db->put(key,value);
				}
			private:
				bool m_good;
				std::unique_ptr<db> m_db;
		};
		struct server {
				server(): m_good(false) {}

				~server() {
					m_good = false;
					if(m_db) {
						m_db.release();
					}
				}
				server(const char* file,const char* dbi_name) {
					init(file,dbi_name,MDB_WRITEMAP,0666);
				}
				bool init(const char* file, const char* dbi_name,int flags, int perms) {
					m_db = std::make_unique<db>(file,dbi_name,flags,perms,true);
					m_good = m_db->good();
					return m_good;
				}
				inline bool good() const {
					return m_db->good();
				}
				inline int get(const std::string& key,std::string& value) {
					return m_db->get(key,value);
				}
				inline int put(const std::string& key,const std::string& value) {
					return m_db->put(key,value);
				}
			private:
				bool m_good;
				std::unique_ptr<db> m_db;
		};

		struct resolver {
				resolver(const std::string& file,const std::string& db_name) {
					m_db = std::make_unique<lmdb::client>(file,db_name);
				}
				~resolver() {

				}
				std::string get(const std::string& host) {
					std::string ip;
					if(m_db->good() == false) {
						std::cerr << "Cannot open lmdb handle!\n";
						return "";
					}
				}
			private:
				std::unique_ptr<lmdb::client> m_db;
		};
	}; /* end lmdb namespace */
}; /* End namespace */
#endif
