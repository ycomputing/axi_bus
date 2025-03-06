#ifndef __AXI_OUTSTANDING_H__
#define __AXI_OUTSTANDING_H__

#include "axi_param.h"

class axi_outstanding
{
public:
	std::unordered_map<uint32_t, axi_trans_t> map;
	std::mutex mutex;
	
	static const int FAIL = 0;
	static const int OK = 1;
	static const int OK_LAST = 2;
	static const int NO_SUCH_ID = 3;
	static const int PREMATURE_LAST = 4;
	static const int OVERFLOW = 5;

	std::string name;
	bool is_write;

	bool create(axi_bus_info_t& info);
	bool remove(uint32_t id);
	int update(axi_bus_info_t& info);
	bool is_id_present(uint32_t id);
	int size_vacant();
	void lock();
	void unlock();
	void dump();
	std::unordered_map<uint32_t, axi_trans_t>::iterator find_by_addr(uint64_t addr);
	std::unordered_map<uint32_t, axi_trans_t>::iterator find_by_id(uint32_t id);
	void log(std::string source, std::string action, std::string detail);
};
#endif