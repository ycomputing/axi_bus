#include <systemc>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <map>
#include <mutex>
#include <unordered_map>


using namespace sc_core;
using namespace sc_dt;

#include "axi_param.h"
#include "axi_outstanding.h"
#include "axi_bus.h"

// When you want to see outstanding dump
#define DEBUG_AXI_OUTSTANDING

bool axi_outstanding::create(axi_bus_info_t& info)
{
	std::string log_detail;

	axi_trans_t trans;
	if (is_id_present(info.id))
	{
		// Duplicate ID
		log(__FUNCTION__, "DUPLICATE", bus_info_to_string(info));
		dump();
		return false;
	}

	if (map.size() >= AXI_BUS_OUTSTANDING_MAX)
	{
		// Too many outstanding now
		log(__FUNCTION__, "TOO MANY", bus_info_to_string(info));
		return false;
	}

	trans.addr = info.addr;
	trans.length = info.len + 1;
	trans.progress = 0;
	trans.is_write = is_write;
	map[info.id] = trans;
	log_detail = "concurrent=" + std::to_string(map.size());
	log_detail += ", id=" + std::to_string(info.id) + ", " + transaction_to_string(trans);
	log(__FUNCTION__, "CREATE OUTSTANDING", log_detail);
	dump();
	return true;
}

bool axi_outstanding::is_id_present(uint32_t id)
{
	return map.find(id) != map.end();
}

std::unordered_map<uint32_t, axi_trans_t>::iterator axi_outstanding::find_by_addr(uint64_t addr)
{
	std::unordered_map<uint32_t, axi_trans_t>::iterator iter;
	axi_trans_t trans;
	for (iter = map.begin(); iter != map.end(); iter ++)
	{
		trans = iter->second;

		if (addr == trans.addr)
		{
			return iter;
		}
	}
	return map.end();
}

std::unordered_map<uint32_t, axi_trans_t>::iterator axi_outstanding::find_by_id(uint32_t id)
{
	return map.find(id);
}

bool axi_outstanding::remove(uint32_t id)
{
	std::string log_detail;

	auto iter = map.find(id);
	if (iter == map.end())
	{
		// No such ID
		log(__FUNCTION__, "NO ID", std::to_string(id));
		dump();
		return false;
	}

	map.erase(iter);

	log_detail = "size=" + std::to_string(map.size());
	log_detail += ", id=" + std::to_string(id);
	log(__FUNCTION__, "REMOVE", log_detail);
	dump();
	return true;
}


// returns LAST_OK when 100% progress is made.

int axi_outstanding::update(axi_bus_info_t& info)
{
	std::string log_detail;

	auto iter = map.find(info.id);
	if (iter == map.end())
	{
		// Nothing in outstanding for that id
		log(__FUNCTION__, "NO ID", bus_info_to_string(info));
		dump();
		return NO_SUCH_ID;
	}

	axi_trans_t& trans = iter->second;

	if (trans.progress >= trans.length)
	{
		log(__FUNCTION__, "OVERFLOW", bus_info_to_string(info));
		dump();
		return OVERFLOW;
	}

	if (info.is_last && (trans.progress != trans.length - 1))
	{
		// We got last data when there must be more
		log(__FUNCTION__, "PREMATURE LAST", bus_info_to_string(info));
		dump();
		return PREMATURE_LAST;
	}

	// be careful to update original data
	trans.data[trans.progress] = info.data;
	trans.progress ++;

	log_detail = "progress=" + std::to_string(trans.progress) + "/"
			+ std::to_string(trans.length) + ", " + bus_info_to_string(info);

	if (info.is_last)
	{
		log(__FUNCTION__, "LAST ONE", log_detail);
		return OK_LAST;
	}

	log(__FUNCTION__, "PLUS ONE", log_detail);
	return OK;
}

int axi_outstanding::size_vacant()
{
	int size = map.size();
	return AXI_BUS_OUTSTANDING_MAX - size;
}

void axi_outstanding::lock()
{
	mutex.lock();
}

void axi_outstanding::unlock()
{
	mutex.unlock();
}

void axi_outstanding::dump()
{

#ifdef DEBUG_AXI_OUTSTANDING
	uint32_t id;
	std::string out;
	axi_trans_t trans;

	out = "----------------------------------------------- outstanding dump begin\n";
	for (auto iter: map)
	{
		id = iter.first;
		trans = iter.second;
		out += ">>>>id=" + std::to_string(id) + ", " + transaction_to_string(trans);
		out += "\n";
	}
	out += "----------------------------------------------- outstanding dump end\n";

	std::cout << out;
#endif

	return;
}

void axi_outstanding::log(std::string source, std::string action, std::string detail)
{
	std::string sep = ":";
	std::string log_source = "OUTSTANDING" + sep + name + sep + source;
	AXI_BUS::log(log_source, action, detail);
}