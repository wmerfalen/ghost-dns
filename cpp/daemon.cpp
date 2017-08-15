#define GHOSTDNS_USE_SHM_CONFIG
#include <memory>
#define GDNS_STL
#include "conf.hpp"

int main(int argc,char** argv){
	/** Parse config file */
	gdns::conf<std::string> c;
	if(c.parse() < 0){
		std::cerr << "Unable to parse config file!\n";
		return 1;
	}
	c.dump_list();
	/** Store entries in lmdb */
	std::unique_ptr<gdns::lmdb::server> db = std::make_unique<gdns::lmdb::server>("/tmp/ghostdns2","ghostdns2");
	if(db->good() == false){
		std::cerr << "failed to initialize lmdb\n";
		return 1;
	}
	std::string value = "bar";
	int ret = db->get("foo",value);
	std::cout << "ret: " << ret << "\n";
	if(ret > 0){
		std::cout << "foo: " << value << "\n";
	}

	ret = db->put("foo","bar full");
	std::cout << "ret: " << ret << "\n";
	if(ret > 0){
		db->get("foo",value);
		std::cout << "foo: " << value << "\n";
	}
	/** Make lmdb entries read-only */

	/** idle forever :) */
    return 0;
}
