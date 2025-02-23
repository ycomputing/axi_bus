#include <algorithm>
#include <systemc>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <map>

using namespace sc_core;
using namespace sc_dt;

#include "axi_subordinate.h"

#define NANOSECONDS_PER_SECOND (1000 * 1000 * 1000)
#define AXI_SUBORDINATE_READ_LATENCY_NS 2
#define AXI_SUBORDINATE_WRITE_LATENCY_NS 300

void AXI_SUBORDINATE::thread_reader()
{
	while(true)
	{
		fifo_reader();
	}
}

void AXI_SUBORDINATE::thread_writer()
{
	while(true)
	{
		fifo_writer();
		wait();
	}
}

void AXI_SUBORDINATE::fifo_reader()
{
	std::string log_action;
	std::string log_detail;

	axi_trans_t trans;
	int latency_ns = 0;
	uint64_t stamp_schedule_ns;
	uint64_t stamp_now_ns;

	// Receive incoming requests. This is a blocking read.
	trans = request.read();
	log(__FUNCTION__, "GOT_REQUEST", AXI_BUS::transaction_to_string(trans));

	latency_ns = get_latency_ns(trans);
	// +0.5 is needed for rounding
	stamp_now_ns = sc_time_stamp().to_seconds() * NANOSECONDS_PER_SECOND + 0.5;
	stamp_schedule_ns = stamp_now_ns + latency_ns;
	mutex_q.lock();
	event_something_to_send.notify (latency_ns, SC_NS);
	q_send.push(when_trans_t(stamp_schedule_ns, trans));
	mutex_q.unlock();

	log_detail = "scheduled=" + std::to_string(stamp_schedule_ns);
	log_detail = ", latency=" + std::to_string(latency_ns);
	log_detail += ", " + AXI_BUS::transaction_to_string(trans);
	log(__FUNCTION__, "SCHEDULE_RESPONSE", log_detail);
}

void AXI_SUBORDINATE::fifo_writer()
{
	std::string log_action;
	std::string log_detail;

	uint64_t stamp_schedule_ns;
	uint64_t stamp_now_ns;
	axi_trans_t trans;

	mutex_q.lock();
	if (q_send.empty())
	{
		log(__FUNCTION__, "EMPTY_QUEUE", "");
		mutex_q.unlock();
		return;
	}

	when_trans_t when_trans = q_send.top();
	stamp_schedule_ns = when_trans.stamp;
	trans = when_trans.trans;
	// +0.5 is needed for rounding
	stamp_now_ns = sc_time_stamp().to_seconds() * NANOSECONDS_PER_SECOND + 0.5;

	if (stamp_now_ns < stamp_schedule_ns)
	{
		log(__FUNCTION__, "WAITING", "stamp_now=" + std::to_string(stamp_now_ns) + ", stamp_schedule=" + std::to_string(stamp_schedule_ns));
		mutex_q.unlock();
		return;
	}

	q_send.pop();
	mutex_q.unlock();

	uint64_t amount_addr_inc = DATA_WIDTH / 8;
	for (int i = 0; i < trans.length; i ++)
	{
		uint64_t addr = trans.addr + amount_addr_inc * i;

		if (trans.is_write)
		{
			map_memory[addr] = trans.data[i];
		}
		else
		{
			auto iter = map_memory.find(addr);
			if (iter != map_memory.end())
			{
				trans.data[i] = map_memory[addr];
			}
			else
			{
				SC_REPORT_FATAL("Address out of range", AXI_BUS::transaction_to_string(trans).c_str());
			}
		}
	}

	response.write(trans);
	log(__FUNCTION__, "SENT_RESPONSE", AXI_BUS::transaction_to_string(trans));
}

int AXI_SUBORDINATE::get_latency_ns(axi_trans_t trans)
{
	int latency_by_address = 0;
	int latency_by_access_type = 0;
	int latency_total_ns = 0;

	if(trans.is_write)
	{
		latency_by_access_type = AXI_SUBORDINATE_WRITE_LATENCY_NS;
	}
	else
	{
		latency_by_access_type = AXI_SUBORDINATE_READ_LATENCY_NS;
	}

	if (trans.addr < 0x8000100010001000)
	{
		latency_by_address = 10;
	}
	else
	{
		latency_by_address = 10000;
	}

	latency_total_ns = latency_by_access_type + latency_by_address;
	return latency_total_ns;
}
void AXI_SUBORDINATE::read_memory_csv()
{
	map_memory.clear();

	std::ifstream f(filename_memory);
	if (!f.is_open())
	{
		std::cerr << "Error: could not open " << filename_memory << std::endl;
		return;
	}

	int line_number = 0;
	std::string line;
	while (std::getline(f, line))
	{
		std::tuple<uint64_t, bus_data_t> row;
		std::istringstream iss(line);
		uint64_t address;
		bus_data_t data;
		std::string token1, token2;
		std::getline(iss, token1, ',');
		std::getline(iss, token2, ',');
		if (token1.empty() || token2.empty())
		{
			std::cerr << "Error: invalid format in " << filename_memory << std::endl;
			std::cerr << "At line (" << line_number << "): " << line << std::endl;
			continue;
		}
		address = address_from_hex_string(token1);
		data = bus_data_from_hex_string(token2);
		map_memory[address] = data;
		line_number ++;
	}
}

void AXI_SUBORDINATE::write_memory_csv(const char *filename)
{
	std::ofstream f(filename);
	if (!f.is_open())
	{
		std::cerr << "Error: could not open " << filename << std::endl;
		return;
	}

	std::map<uint64_t, bus_data_t> map_ordered(map_memory.begin(), map_memory.end());
	for (auto row : map_ordered)
	{
		uint64_t address = std::get<0>(row);
		bus_data_t data = std::get<1>(row);
		f << "0x" << std::setfill('0') << std::setw(ADDR_WIDTH / 4) << std::hex << address;
		f << ","  << std::setfill('0') << std::setw(DATA_WIDTH / 4) << data.to_string(SC_HEX) << std::endl;
	}
}

void AXI_SUBORDINATE::log(std::string source, std::string action, std::string detail)
{
	std::string sep = ":";
	std::string log_source = "SUBORDINATE" + sep + name() + sep + source;
	AXI_BUS::log(log_source, action, detail);
}
