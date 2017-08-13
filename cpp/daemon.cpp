#define GHOSTDNS_USE_SHM_CONFIG
#include "liblmdb/lmdb.h"
#include <memory>
#define GDNS_STL
#include "conf.hpp"
using namespace gdns;

int main(int argc,char** argv){
	/** Create lmdb handle */
	MDB_env* mdb;
	if(mdb_env_create(&mdb) < 0){
		std::cerr << "[lmdb] failed to open new environment\n";
		return 1;
	}

	/** Open the lmdb database handle */
	if(mdb_env_open(mdb,"/tmp/ghostdns",MDB_WRITEMAP | MDB_NOLOCK,0666) < 0){
		std::cerr << "[lmdb] failed to open directory '/tmp/ghostdns' make sure it exists!\n";
		return 1;
	}
	/** Parse config file */
	gdns::conf<std::string> c;
	if(c.parse() < 0){
		std::cerr << "Unable to parse config file!\n";
		return 1;
	}

	/** Store entries in lmdb */
	MDB_txn *txn;
	MDB_dbi dbi;
	{
		/** Begin transaction */
		if(mdb_txn_begin(mdb,NULL,MDB_WRITEMAP | MDB_NOLOCK,&txn) < 0){
			std::cerr << "[lmdb] failed to open transaction!\n";
			return 1;
		}
		/** Open the database */
		if(mdb_dbi_open(txn,"ghostdns",MDB_CREATE,&dbi) < 0){
			std::cerr << "[lmdb] failed to open dbi connection\n";
			return 1;
		}

	}

	/** Make lmdb entries read-only */
	/** idle forever :) */
    return 0;
}
